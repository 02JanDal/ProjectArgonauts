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

#include "MainWindow.h"

#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QSignalMapper>
#include <QSettings>
#include <QApplication>
#include <QUndoStack>
#include <QUndoGroup>
#include <QUndoView>
#include <QMenuBar>
#include <QToolBar>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QSaveFile>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QDesktopServices>
#include <QStackedLayout>
#include <QDockWidget>
#include <QScrollBar>

#include "Document.h"
#include "TextEditor.h"
#include "GraphicalEditor.h"
#include "DiagnosticsModel.h"

#include "AboutDialog.h"
#include "HelpDialog.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	m_undos = new QUndoGroup(this);
	m_textualUndoStack = new QUndoStack(this);
	m_graphicalUndoStack = new QUndoStack(this);
	m_undos->addStack(m_textualUndoStack);
	m_undos->addStack(m_graphicalUndoStack);

	m_document = new Document(this);
	connect(m_document, &Document::modificationChanged, this, &MainWindow::setWindowModified);
	connect(m_document, &Document::nameChanged, this, &MainWindow::updateWindowTitle);
	connect(m_document, &Document::fileNameChanged, this, &MainWindow::addDocumentToRecent);

	m_textEditor = new TextEditor(m_document, m_textualUndoStack, this);
	m_textEditor->setTabStopWidth(QFontMetrics(m_textEditor->font()).width(' ') * 4);
	m_graphicalEditor = new GraphicalEditor(m_document, m_graphicalUndoStack, this);

	m_layout = new QStackedLayout;
	m_layout->addWidget(m_textEditor);
	m_layout->addWidget(m_graphicalEditor);
	setCentralWidget(new QWidget);
	centralWidget()->setLayout(m_layout);

	setupDockWidgets();
	setupMenusAndActions();
	setupToolBar();

	textualMode();
	updateWindowTitle();

	QSettings settings;
	settings.beginGroup("Editor");
	settings.beginGroup("MainWindow");
	if (settings.contains("Geometry")) {
		restoreGeometry(settings.value("Geometry").toByteArray());
		restoreState(settings.value("State").toByteArray());
	} else {
		resize(640, 480);
	}
}

MainWindow::~MainWindow()
{
	QSettings settings;
	settings.beginGroup("Editor");
	settings.beginGroup("MainWindow");
	settings.setValue("Geometry", saveGeometry());
	settings.setValue("State", saveState());
}

bool MainWindow::newDocument()
{
	if (m_document->isModified()) {
		const int ret = QMessageBox::question(this, tr("Discard changes?"), tr("The current document has unsaved changes."), QMessageBox::Discard | QMessageBox::Save | QMessageBox::Cancel, QMessageBox::Cancel);
		if (ret == QMessageBox::Save) {
			if (!saveDocument()) {
				return false;
			}
		} else if (ret == QMessageBox::Cancel) {
			return false;
		}
	}

	m_document->clear();
	return true;
}
bool MainWindow::openDocument(const QString &filename)
{
	if (!newDocument()) {
		return false;
	}

	if (filename.isNull()) {
		const QString file = QFileDialog::getOpenFileName(this, tr("Open Argonauts File"), getSetting("LastDirectory", QDir::homePath()).toString(), tr("Argonaut files (*arg)"));
		if (file.isEmpty()) {
			return false;
		}
		return openDocument(filename);
	}

	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to open file %1 for reading: %2").arg(filename, file.errorString()));
		return false;
	}
	m_document->setFileName(filename);
	m_document->setPlainText(QString::fromUtf8(file.readAll()));
	QTextCursor cursor = m_textEditor->textCursor();
	cursor.movePosition(QTextCursor::Start);
	m_textEditor->setTextCursor(cursor);
	return true;
}
bool MainWindow::saveDocument()
{
	if (m_document->fileName().isEmpty()) {
		return saveDocumentAs();
	}

	QSaveFile file(m_document->fileName());
	if (!file.open(QFile::WriteOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to open %1 for writing: %2").arg(file.fileName(), file.errorString()));
		return false;
	}
	file.write(m_document->toPlainText().toUtf8());
	if (file.commit()) {
		m_document->save();
		return true;
	} else {
		QMessageBox::warning(this, tr("Error"), tr("Unable to write to %1: %2").arg(file.fileName(), file.errorString()));
		return false;
	}
}
bool MainWindow::saveDocumentAs()
{
	const QString filename = QFileDialog::getSaveFileName(this, tr("Save Argonauts File"), getSetting("LastDirectory", QDir::homePath()).toString(), tr("Argonauts file (*.arg)"));
	if (filename.isEmpty()) {
		return false;
	}

	m_document->setFileName(filename);
	return saveDocument();
}

void MainWindow::printText()
{
	QPrintPreviewDialog dlg(this);
	connect(&dlg, &QPrintPreviewDialog::paintRequested, m_document, &QTextDocument::print);
	dlg.exec();
}
void MainWindow::printDiagram()
{
	QPrintPreviewDialog dlg(this);
	connect(&dlg, &QPrintPreviewDialog::paintRequested, [this](QPrinter *printer)
	{
		QPainter painter(printer);
		m_graphicalEditor->render(&painter,
								  QRectF(0, 0, printer->width(), printer->height()));
	});
	dlg.exec();
}

void MainWindow::reportBugs()
{
	QDesktopServices::openUrl(QUrl("https://github.com/02JanDal/ProjectArgonauts/issues"));
}
void MainWindow::github()
{
	QDesktopServices::openUrl(QUrl("https://github.com/02JanDal/ProjectArgonauts"));
}
void MainWindow::help()
{
	HelpDialog(this).exec();
}
void MainWindow::about()
{
	AboutDialog(this).exec();
}

void MainWindow::textualMode()
{
	m_textModeActions->setEnabled(true);
	m_graphicalModeActions->setEnabled(false);
	m_undos->setActiveStack(m_textualUndoStack);
	m_layout->setCurrentWidget(m_textEditor);
}
void MainWindow::graphicalMode()
{
	m_textModeActions->setEnabled(false);
	m_graphicalModeActions->setEnabled(true);
	m_undos->setActiveStack(m_graphicalUndoStack);
	m_layout->setCurrentWidget(m_graphicalEditor);
}

void MainWindow::zoomOut()
{

}
void MainWindow::zoomIn()
{

}
void MainWindow::zoomFit()
{

}

void MainWindow::updateWindowTitle()
{
	if (m_document->name().isEmpty()) {
		setWindowTitle(tr("New[*]"));
	} else {
		setWindowTitle(m_document->name() + "[*]");
	}
}
void MainWindow::updateRecentDocuments()
{
	for (QAction *action : m_recentDocumentActions) {
		m_recentDocumentsMapper->removeMappings(action);
		delete action;
	}
	m_recentDocumentActions.clear();

	const QStringList paths = getSetting("RecentDocuments").toStringList();
	for (const QString &path : paths) {
		QAction *action = m_openRecentMenu->addAction(path, m_recentDocumentsMapper, SLOT(map()));
		m_recentDocumentsMapper->setMapping(action, path);
		m_recentDocumentActions.append(action);
	}
}

void MainWindow::addDocumentToRecent(const QString &document)
{
	QSettings settings;
	settings.beginGroup("Editor");
	QStringList docs = settings.value("RecentDocuments").toStringList();
	docs.removeAll(document);
	docs.prepend(document);
	settings.setValue("RecentDocuments", docs);
	settings.sync();
	updateRecentDocuments();
}

QVariant MainWindow::getSetting(const QString &name, const QVariant &def) const
{
	QSettings settings;
	settings.beginGroup("Editor");
	return settings.value(name, def);
}

void MainWindow::setupDockWidgets()
{
	m_undoViewDockWidget = new QDockWidget(tr("Undo && Redo"), this);
	{
		m_undoViewDockWidget->setWidget(new QUndoView(m_undos));
		m_undoViewDockWidget->setObjectName("DockWidgetUndoRedo");
		m_undoViewDockWidget->hide();
		addDockWidget(Qt::LeftDockWidgetArea, m_undoViewDockWidget);
	}
	m_diagnosticsDockWidget = new QDockWidget(tr("Diagnostics"), this);
	{
		QListView *view = new QListView;
		view->setModel(DiagnosticsSortFilterProxyModel::create(m_document, view));
		connect(view, &QListView::doubleClicked, [this](const QModelIndex &index)
		{
			if (!index.isValid()) {
				return;
			}
			QTextCursor cursor(m_document);
			cursor.setPosition(index.data(DiagnosticsModel::StartRole).toInt());
			cursor.setPosition(index.data(DiagnosticsModel::EndRole).toInt(), QTextCursor::KeepAnchor);
			m_textEditor->setTextCursor(cursor);
		});
		m_diagnosticsDockWidget->setWidget(view);
		m_diagnosticsDockWidget->setObjectName("DockWidgetDiagnostics");
		addDockWidget(Qt::BottomDockWidgetArea, m_diagnosticsDockWidget);
	}
}

void MainWindow::setupMenusAndActions()
{
	m_fileMenu = new QMenu(tr("&File"), this);
	{
		m_newAction = m_fileMenu->addAction(QIcon::fromTheme("document-new"), tr("&New"), this, SLOT(newDocument()), QKeySequence::New);
		m_openAction = m_fileMenu->addAction(QIcon::fromTheme("document-open"), tr("&Open"), this, SLOT(openDocument()), QKeySequence::Open);
		m_openRecentMenu = m_fileMenu->addMenu(QIcon::fromTheme("document-open-recent"), tr("Open &Recent"));
		{
			m_recentDocumentsMapper = new QSignalMapper(this);
			updateRecentDocuments();
		}
		m_saveAction = m_fileMenu->addAction(QIcon::fromTheme("document-save"), tr("&Save"), this, SLOT(saveDocument()), QKeySequence::Save);
		m_saveAsAction = m_fileMenu->addAction(QIcon::fromTheme("document-save-as"), tr("Save &As"), this, SLOT(saveDocumentAs()), QKeySequence::SaveAs);
		m_fileMenu->addSeparator();
		m_printMenu = m_fileMenu->addMenu(QIcon::fromTheme("document-print"), tr("&Print"));
		{
			m_printTextAction = m_printMenu->addAction(tr("&Text"), this, SLOT(printText()));
			m_printDiagramAction = m_printMenu->addAction(tr("&Diagram"), this, SLOT(printDiagram()));
		}
		m_fileMenu->addSeparator();
		m_quitAction = m_fileMenu->addAction(QIcon::fromTheme("application-exit"), tr("&Exit"), qApp, SLOT(quit()), QKeySequence::Quit);
	}

	m_editMenu = new QMenu(tr("&Edit"), this);
	{
		m_editMenu->addAction(m_undoAction = m_undos->createUndoAction(this));
		m_undoAction->setIcon(QIcon::fromTheme("edit-undo"));
		m_undoAction->setShortcut(QKeySequence::Undo);
		m_editMenu->addAction(m_redoAction = m_undos->createRedoAction(this));
		m_redoAction->setIcon(QIcon::fromTheme("edit-redo"));
		m_redoAction->setShortcut(QKeySequence::Redo);

		m_editMenu->addSeparator();
		m_cutAction = m_editMenu->addAction(QIcon::fromTheme("edit-cut"), tr("Cu&t"), m_textEditor, SLOT(cut()), QKeySequence::Cut);
		m_copyAction = m_editMenu->addAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), m_textEditor, SLOT(copy()), QKeySequence::Copy);
		m_pasteAction = m_editMenu->addAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), m_textEditor, SLOT(paste()), QKeySequence::Paste);
		connect(m_textEditor, &QPlainTextEdit::copyAvailable, m_copyAction, &QAction::setEnabled);

		m_editMenu->addSeparator();
		m_selectAll = m_editMenu->addAction(QIcon::fromTheme("select-all"), tr("Select &All"), m_textEditor, SLOT(selectAll()), QKeySequence::SelectAll);
	}

	m_viewMenu = new QMenu(tr("&View"), this);
	{
		m_textualModeAction = m_viewMenu->addAction(QIcon::fromTheme("applications-office"), tr("&Textual Mode"), this, SLOT(textualMode()));
		m_graphicalModeAction = m_viewMenu->addAction(QIcon::fromTheme("applications-graphics"), tr("&Grapical Mode"), this, SLOT(graphicalMode()));
		m_textualModeAction->setCheckable(true);
		m_graphicalModeAction->setCheckable(true);
		m_textualModeAction->setChecked(true);
		QActionGroup *modeGroup = new QActionGroup(this);
		modeGroup->setExclusive(true);
		modeGroup->addAction(m_textualModeAction);
		modeGroup->addAction(m_graphicalModeAction);

		m_viewMenu->addSeparator();
		m_dockWidgetsMenu = m_viewMenu->addMenu(QIcon::fromTheme("window-duplicate"), tr("&Dock Widgets"));
		for (QDockWidget *widget : {m_undoViewDockWidget}) {
			m_dockWidgetsMenu->addAction(widget->toggleViewAction());
		}

		m_viewMenu->addSeparator();
		m_zoomInAction = m_viewMenu->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom &Out"), this, SLOT(zoomOut()), QKeySequence::ZoomOut);
		m_zoomOutAction = m_viewMenu->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom &In"), this, SLOT(zoomIn()), QKeySequence::ZoomIn);
		m_zoomFitAction = m_viewMenu->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Zoom &Fit"), this, SLOT(zoomFit()));
	}

	m_helpMenu = new QMenu(tr("&Help"), this);
	{
		m_reportBugsAction = m_helpMenu->addAction(QIcon::fromTheme("tools-report-bug"), tr("Report &Bugs"), this, SLOT(reportBugs()));
		m_githubAction = m_helpMenu->addAction(QIcon::fromTheme("github-repo"), tr("&GitHub"), this, SLOT(github()));
		m_helpAction = m_helpMenu->addAction(QIcon::fromTheme("help-contents"), tr("&Help"), this, SLOT(help()), QKeySequence::HelpContents);
		m_aboutAction = m_helpMenu->addAction(QIcon::fromTheme("help-about"), tr("&About"), this, SLOT(about()));
		m_aboutQtAction = m_helpMenu->addAction(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), qApp, SLOT(aboutQt()));
	}

	m_textModeActions = new QActionGroup(this);
	m_textModeActions->addAction(m_cutAction);
	m_textModeActions->addAction(m_copyAction);
	m_textModeActions->addAction(m_pasteAction);
	m_textModeActions->addAction(m_selectAll);

	m_graphicalModeActions = new QActionGroup(this);
	m_graphicalModeActions->addAction(m_zoomInAction);
	m_graphicalModeActions->addAction(m_zoomOutAction);
	m_graphicalModeActions->addAction(m_zoomFitAction);

	menuBar()->addMenu(m_fileMenu);
	menuBar()->addMenu(m_editMenu);
	menuBar()->addMenu(m_viewMenu);
	menuBar()->addMenu(m_helpMenu);
}

void MainWindow::setupToolBar()
{
	m_toolBar = addToolBar(tr("Toolbar: Main"));
	m_toolBar->setObjectName("ToolBarMain");
	m_toolBar->addAction(m_newAction);
	m_toolBar->addAction(m_openAction);
	m_toolBar->addAction(m_saveAction);
	m_toolBar->addSeparator();
	m_toolBar->addAction(m_undoAction);
	m_toolBar->addAction(m_redoAction);
	m_toolBar->addSeparator();
	m_toolBar->addAction(m_textualModeAction);
	m_toolBar->addAction(m_graphicalModeAction);
}

#include "MainWindow.moc"
