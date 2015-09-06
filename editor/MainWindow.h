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

#include <QMainWindow>
#include <QVariant>

class Document;
class TextEditor;
class GraphicalEditor;
class QSignalMapper;
class QUndoStack;
class QUndoGroup;
class QStackedLayout;
class QActionGroup;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

public slots:
	bool newDocument();
	bool openDocument(const QString &filename = QString());
	bool saveDocument();
	bool saveDocumentAs();
	void printText();
	void printDiagram();

	void reportBugs();
	void github();
	void help();
	void about();

	void textualMode();
	void graphicalMode();
	void zoomOut();
	void zoomIn();
	void zoomFit();

private slots:
	void updateWindowTitle();

private:
	void updateRecentDocuments();
	void addDocumentToRecent(const QString &document);
	QVariant getSetting(const QString &name, const QVariant &def = QVariant()) const;

private: // current state
	Document *m_document;
	QUndoGroup *m_undos;
	QUndoStack *m_textualUndoStack;
	QUndoStack *m_graphicalUndoStack;

private: // ui
	TextEditor *m_textEditor;
	GraphicalEditor *m_graphicalEditor;
	QStackedLayout *m_layout;

	void setupDockWidgets();
	QDockWidget *m_undoViewDockWidget;
	QDockWidget *m_diagnosticsDockWidget;

	void setupMenusAndActions();
	// top-level
	QMenu *m_fileMenu;
	QMenu *m_editMenu;
	QMenu *m_viewMenu;
	QMenu *m_helpMenu;

	// sub-actions/menus of fileMenu
	QAction *m_newAction;
	QAction *m_openAction;
	QMenu *m_openRecentMenu;
	QVector<QAction *> m_recentDocumentActions;
	QSignalMapper *m_recentDocumentsMapper;
	QAction *m_saveAction;
	QAction *m_saveAsAction;
	QMenu *m_printMenu;
	QAction *m_printTextAction;
	QAction *m_printDiagramAction;
	QAction *m_quitAction;

	// sub-actions/menus of editMenu
	QAction *m_undoAction;
	QAction *m_redoAction;
	QAction *m_cutAction;
	QAction *m_copyAction;
	QAction *m_pasteAction;
	QAction *m_selectAll;

	// sub-actions/menus of viewMenu
	QAction *m_textualModeAction;
	QAction *m_graphicalModeAction;
	QMenu *m_dockWidgetsMenu;
	QAction *m_zoomOutAction;
	QAction *m_zoomInAction;
	QAction *m_zoomFitAction;

	// sub-actions/menus of helpMenu
	QAction *m_reportBugsAction;
	QAction *m_githubAction;
	QAction *m_helpAction;
	QAction *m_aboutAction;
	QAction *m_aboutQtAction;

	void setupToolBar();
	QToolBar *m_toolBar;
	QActionGroup *m_textModeActions;
	QActionGroup *m_graphicalModeActions;
};
