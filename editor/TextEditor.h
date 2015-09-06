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

#pragma once

#include <QPlainTextEdit>

class Document;
class QUndoStack;

class TextEditor : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit TextEditor(Document *document, QUndoStack *undoStack, QWidget *parent = nullptr);

private:
	Document *m_document;
	QUndoStack *m_undoStack;

	bool event(QEvent *e) override;

	QString toolTipFor(const QPoint &pos) const;

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &rect, const int dy);

private:
	friend class LineNumberArea;
	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();
	void resizeEvent(QResizeEvent *e) override;
	QWidget *m_lineNumberArea;
};
