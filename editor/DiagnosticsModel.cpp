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

#include "DiagnosticsModel.h"

#include <QIcon>

DiagnosticsModel::DiagnosticsModel(Document *document, QObject *parent)
	: QAbstractListModel(parent), m_document(document)
{
	connect(m_document, &Document::diagnosticAdded, this, &DiagnosticsModel::diagnosticAdded);
	connect(m_document, &Document::diagnosticRemoved, this, &DiagnosticsModel::diagnosticRemoved);
	connect(m_document, &Document::diagnosticsReset, this, &DiagnosticsModel::diagnosticsReset);
}

int DiagnosticsModel::rowCount(const QModelIndex &) const
{
	return m_diagnostics.size();
}
QVariant DiagnosticsModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_diagnostics.size()) {
		return QVariant();
	}

	const Document::Diagnostic &diagnostic = m_diagnostics.at(index.row());
	switch (role) {
	case RawRole: return QVariant::fromValue(diagnostic);
	case StartRole: return diagnostic.start;
	case EndRole: return diagnostic.end;
	case StringRole: return diagnostic.string;

	case Qt::DisplayRole: return diagnostic.string;
	case Qt::DecorationRole:
		return QIcon::fromTheme(diagnostic.icon());
	case Qt::BackgroundRole:
		switch (diagnostic.type) {
		case Document::Diagnostic::Error: return QBrush(Qt::red);
		case Document::Diagnostic::Warning: return QBrush(Qt::yellow);
		case Document::Diagnostic::Notice: return QBrush(Qt::blue);
		}
	default:
		return QVariant();
	}
}

void DiagnosticsModel::diagnosticAdded(const Document::Diagnostic &diagnostic)
{
	beginInsertRows(QModelIndex(), m_diagnostics.size(), m_diagnostics.size());
	m_diagnostics.append(diagnostic);
	endInsertRows();
}
void DiagnosticsModel::diagnosticRemoved(const Document::Diagnostic &diagnostic)
{
	const int index = m_diagnostics.indexOf(diagnostic);
	beginRemoveRows(QModelIndex(), index, index);
	m_diagnostics.removeAt(index);
	endRemoveRows();
}
void DiagnosticsModel::diagnosticsReset()
{
	beginResetModel();
	m_diagnostics.clear();
	endResetModel();
}

DiagnosticsSortFilterProxyModel::DiagnosticsSortFilterProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent)
{}

DiagnosticsSortFilterProxyModel *DiagnosticsSortFilterProxyModel::create(Document *document, QObject *parent)
{
	DiagnosticsSortFilterProxyModel *proxy = new DiagnosticsSortFilterProxyModel(parent);
	proxy->setSourceModel(new DiagnosticsModel(document, proxy));
	return proxy;
}

void DiagnosticsSortFilterProxyModel::setMinType(const Document::Diagnostic::Type &type)
{
	m_minType = type;
	invalidateFilter();
}

bool DiagnosticsSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	const Document::Diagnostic &diagnostic = sourceModel()->index(sourceRow, 0, sourceParent).data(Qt::UserRole).value<Document::Diagnostic>();
	return diagnostic.type <= m_minType;
}
bool DiagnosticsSortFilterProxyModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
	const Document::Diagnostic &left = sourceLeft.data(Qt::UserRole).value<Document::Diagnostic>();
	const Document::Diagnostic &right = sourceRight.data(Qt::UserRole).value<Document::Diagnostic>();
	if (left.type != right.type) {
		return left.type < right.type;
	} else {
		return left.start < right.start;
	}
}
