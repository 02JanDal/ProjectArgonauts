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

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "Document.h"

class DiagnosticsModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit DiagnosticsModel(Document *document, QObject *parent = nullptr);

	enum
	{
		RawRole = Qt::UserRole,
		StartRole,
		EndRole,
		StringRole
	};

	int rowCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;

private slots:
	void diagnosticAdded(const Document::Diagnostic &diagnostic);
	void diagnosticRemoved(const Document::Diagnostic &diagnostic);
	void diagnosticsReset();

private:
	Document *m_document;
	QVector<Document::Diagnostic> m_diagnostics;
};

class DiagnosticsSortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit DiagnosticsSortFilterProxyModel(QObject *parent = nullptr);

	static DiagnosticsSortFilterProxyModel *create(Document *document, QObject *parent = nullptr);

	void setMinType(const Document::Diagnostic::Type &type);

private:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
	bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;

	Document::Diagnostic::Type m_minType = Document::Diagnostic::Notice;
};
