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

#include <QTextDocument>
#include <QTextCursor>

class Document : public QTextDocument
{
	Q_OBJECT
public:
	explicit Document(QObject *parent = nullptr);

	void setFileName(const QString &filename);
	QString fileName() const { return m_fileName; }
	QString name() const;
	int savedRevision() const { return m_savedRevision; }
	void save();

	void setPlainText(const QString &text);

	class Diagnostic
	{
	public:
		enum Type
		{
			Error,
			Warning,
			Notice
		} type;
		int start, end;
		QString string;

		QString icon() const;

		bool operator==(const Diagnostic &other) const;
	};
	QVector<Diagnostic> diagnosticsFor(const QTextCursor &cursor = QTextCursor()) const;
	QVector<Diagnostic> diagnosticsFor(const QTextBlock &block) const;

	class TypeDefinition
	{
	public:
		enum Type
		{
			Enum,
			Struct,
			Alias,
			Attribute,
			Entry
		} type;

		int start, end;
		QString name;
	};
	TypeDefinition findDefinitionFor(const QString &name) const;

	class TypeUsage
	{
	public:
		int start, end;
		QString name;
	};
	QVector<TypeUsage> findUsagesFor(const QString &name) const;

signals:
	void fileNameChanged(const QString &filename);
	void nameChanged(const QString &name);

	void diagnosticAdded(const Diagnostic &diagnostic);
	void diagnosticRemoved(const Diagnostic &diagnostic);
	void diagnosticsReset();

private:
	QString m_fileName;
	int m_savedRevision;
};

Q_DECLARE_METATYPE(Document::Diagnostic)
