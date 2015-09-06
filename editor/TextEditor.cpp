/*
 * Copyright 2015 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TextEditor.h"

#include <QDebug>
#include <QFontDatabase>
#include <QHelpEvent>
#include <QToolTip>
#include <QPainter>
#include <QTextBlock>
#include <QIcon>

#include "Document.h"

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(TextEditor *editor) : QWidget(editor), m_editor(editor) {}

	QSize sizeHint() const override
	{
		return QSize(m_editor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override
	{
		m_editor->lineNumberAreaPaintEvent(event);
	}

private:
	TextEditor *m_editor;
};

TextEditor::TextEditor(Document *document, QUndoStack *undoStack, QWidget *parent)
	: QPlainTextEdit(parent),
	  m_document(document),
	  m_undoStack(undoStack),
	  m_lineNumberArea(new LineNumberArea(this))
{
	setLineWrapMode(NoWrap);
	setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	setDocument(document);
	connect(document, &Document::nameChanged, this, &TextEditor::setDocumentTitle);

	connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
	connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);
	connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}

bool TextEditor::event(QEvent *e)
{
	if (e->type() == QEvent::ToolTip) {
		QHelpEvent *event = static_cast<QHelpEvent *>(e);
		const QString tooltip = toolTipFor(event->pos());
		if (tooltip.isEmpty()) {
			event->ignore();
		} else {
			QToolTip::showText(event->globalPos(), tooltip, this);
		}
		return true;
	} else {
		return QPlainTextEdit::event(e);
	}
}

QString TextEditor::toolTipFor(const QPoint &pos) const
{
	const QTextCursor cursor = cursorForPosition(pos);
	if (cursor.isNull()) {
		return QString();
	}
	const QVector<Document::Diagnostic> diagnostics = m_document->diagnosticsFor(cursor);
	if (!diagnostics.isEmpty()) {
		return diagnostics.first().string;
	} else {
		return QString();
	}
}

void TextEditor::updateLineNumberAreaWidth(int)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}
void TextEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(QColor(Qt::yellow).lighter(180));
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}
void TextEditor::updateLineNumberArea(const QRect &rect, const int dy)
{
	if (dy) {
		m_lineNumberArea->scroll(0, dy);
	} else {
		m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
	}

	if (rect.contains(viewport()->rect())) {
		updateLineNumberAreaWidth(0);
	}
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(m_lineNumberArea);
	painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(110));

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			const int numberWidth = m_lineNumberArea->width() - (2 + 1 + 15);
			// line number
			painter.setPen(QColor(Qt::darkGray));
			painter.drawText(0, top + 1, numberWidth, fontMetrics().height(),
							 Qt::AlignRight, QString::number(blockNumber + 1));

			// modified marker
			painter.setPen(block.revision() > m_document->savedRevision() ? Qt::red : Qt::darkGreen);
			painter.setBrush(painter.pen().color());
			painter.drawRect(numberWidth + 2, top, 1, fontMetrics().height() + 3);

			// error markers
			QVector<Document::Diagnostic> diagnostics = m_document->diagnosticsFor(block);
			if (!diagnostics.isEmpty()) {
				std::sort(diagnostics.begin(), diagnostics.end(), [](const Document::Diagnostic &a, const Document::Diagnostic &b) { return a.type < b.type; });
				const int side = std::min(12, fontMetrics().height());
				painter.drawPixmap(numberWidth + 2 + 1 + 2, top + std::max(0, fontMetrics().height() - side), side, side,
								   QIcon::fromTheme(diagnostics.first().icon()).pixmap(side, side));
			}
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

int TextEditor::lineNumberAreaWidth()
{
	return 2 + fontMetrics().width(QString::number(qMax(1, blockCount()))) + 2 + 1 + 2 + 13;
}
void TextEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));

}
