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

#include "Document.h"

#include <QFileInfo>
#include <QPlainTextDocumentLayout>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QStack>
#include <QDebug>
#include <memory>

#include "common/Lexer.h"
#include "common/GeneralParser.h"
#include "common/Token.h"
#include "util/Error.h"

using Argonauts::Common::Lexer;
using Argonauts::Common::GeneralParser;
using Argonauts::Common::Token;
using Argonauts::Util::Error;

struct UserData : public QTextBlockUserData
{
	static std::shared_ptr<UserData> getForBlock(const QTextBlock &block)
	{
		UserData *raw = static_cast<UserData *>(block.userData());
		if (raw) {
			return std::shared_ptr<UserData>(raw, [](UserData *) {});
		} else {
			return std::make_shared<UserData>();
		}
	}

	~UserData()
	{
		for (const Document::Diagnostic &diagnostic : diagnostics) {
			emit document->diagnosticRemoved(diagnostic);
		}
	}

	QVector<Document::TypeDefinition> definitions;
	QVector<Document::TypeUsage> usages;

	QVector<Document::Diagnostic> diagnostics;
	void addDiagnostic(const int start, const int end, const QString &string, const Document::Diagnostic::Type type = Document::Diagnostic::Error)
	{
		diagnostics.append(Document::Diagnostic{type, start, end, string});
		emit document->diagnosticAdded(diagnostics.last());
	}

	Document *document = nullptr;
	bool isValid = false;
	UserData *previous = nullptr;
	Lexer::StartState state = Lexer::Normal;
	std::vector<Token> tokens; ///< Tokens in this block
	std::vector<Token> tokenStack; ///< Current token stack, as used by the parser
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
	QTextCharFormat m_commentFormat;
	QTextCharFormat m_keywordFormat;
	QTextCharFormat m_typeFormat;
	QTextCharFormat m_attributeNameFormat;
	QTextCharFormat m_annotationNameFormat;
	QTextCharFormat m_stringFormat;
	QTextCharFormat m_integerFormat;

	explicit SyntaxHighlighter(Document *document)
		: QSyntaxHighlighter(document)
	{
		m_commentFormat.setForeground(Qt::darkGreen);
		m_keywordFormat.setForeground(Qt::darkYellow);
		m_typeFormat.setForeground(Qt::darkMagenta);
		m_attributeNameFormat.setForeground(Qt::darkRed);
		m_annotationNameFormat.setForeground(QColor(Qt::darkBlue).darker(140));
		m_stringFormat.setForeground(Qt::darkGreen);
		m_integerFormat.setForeground(Qt::darkBlue);
	}

	QTextCharFormat makeErrorFormat(const QTextCharFormat &fmt)
	{
		QTextCharFormat newFormat = fmt;
		newFormat.setUnderlineColor(Qt::red);
		newFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
		return newFormat;
	}

	void highlightBlock(const QString &text) override
	{
		const int blockStart = currentBlock().position();

		std::shared_ptr<UserData> previousData = UserData::getForBlock(currentBlock().previous());

		UserData *data = new UserData;
		data->document = static_cast<Document *>(document());
		data->isValid = true;
		data->previous = previousData->isValid ? previousData.get() : nullptr;

		auto defineAnnotation = [this](const Token &token, const std::vector<Token> &stack)
		{
			setFormat(token, m_annotationNameFormat);
			return GeneralParser::defineAnnotation(token, stack);
		};
		auto addToAnnotation = [this](const Token &token, const std::vector<Token> &stack)
		{
			for (int i = (stack.size() - 1); i >= 0; --i) {
				if (stack.at(i).type == Token::Annotation) {
					setFormat(stack.at(i + 2), m_annotationNameFormat);
				}
			}
			return GeneralParser::addToAnnotation(token, stack);
		};
		auto typeDefinition = [data](const std::vector<Token> &stack)
		{
			const Token typeToken = stack.back();
			for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
				if (it->type == Token::Keyword_Struct) {
					data->definitions.append(Document::TypeDefinition{Document::TypeDefinition::Struct, typeToken.offset, typeToken.offset + typeToken.length, QString::fromStdString(typeToken.string())});
				} else if (it->type == Token::Keyword_Enum) {
					data->definitions.append(Document::TypeDefinition{Document::TypeDefinition::Enum, typeToken.offset, typeToken.offset + typeToken.length, QString::fromStdString(typeToken.string())});
				} else if (it->type == Token::Keyword_Using) {
					data->definitions.append(Document::TypeDefinition{Document::TypeDefinition::Alias, typeToken.offset, typeToken.offset + typeToken.length, QString::fromStdString(typeToken.string())});
				} else if (it->type == Token::Struct) {
					data->definitions.append(Document::TypeDefinition{Document::TypeDefinition::Attribute, typeToken.offset, typeToken.offset + typeToken.length, QString::fromStdString(typeToken.string())});
				} else if (it->type == Token::Enum) {
					data->definitions.append(Document::TypeDefinition{Document::TypeDefinition::Entry, typeToken.offset, typeToken.offset + typeToken.length, QString::fromStdString(typeToken.string())});
				} else {
					continue;
				}
				break;
			}
		};
		auto typeUsage = [data](const std::vector<Token> &stack)
		{
			const Token token = stack.back();
			data->usages.append(Document::TypeUsage{token.offset, token.offset + token.length, QString::fromStdString(token.string())});
		};
		auto morphIdentifierToType = [](const Token &token) { return Token(Token::Type, token.string(), token.offset, token.length); };
		auto morphTypeToTypeList = [](const Token &token) { return Token(Token::TypeList, std::string(), token.offset, token.length); };
		auto unexpected = [data](const Token &token)
		{
			if (token.type == Token::Error) {
				data->addDiagnostic(token.offset, token.offset + token.length, QString::fromStdString(token.string()));
			} else {
				data->addDiagnostic(token.offset, token.offset + token.length, SyntaxHighlighter::tr("Unexpected token '%1'").arg(QString::fromStdString(Token::toString(token.type))));
			}
		};

		Lexer lexer;
		lexer.setOffset(currentBlock().position());
		lexer.setState(previousData ? previousData->state : Lexer::Normal);
		data->tokens = lexer.consume(text.toStdString(), std::string(), false);
		data->tokenStack = GeneralParser::parse({&GeneralParser::defineStruct, &GeneralParser::defineStructWithInclude, &GeneralParser::defineEnum, &GeneralParser::defineUsing,
												 defineAnnotation, addToAnnotation, &GeneralParser::defineEnumEntry, &GeneralParser::defineStructAttribute,
												 &GeneralParser::defineStructAttributeWithIndex, &GeneralParser::convertTypeToTypeList, &GeneralParser::mergeTypeListAndType, &GeneralParser::finalizeType},
												{typeDefinition, typeUsage},
												{morphIdentifierToType, morphTypeToTypeList},
												previousData ? previousData->tokenStack : std::vector<Token>(),
												data->tokens,
												unexpected);
		for (const Token &token : data->tokens) {
			switch (token.type) {
			case Token::Comment: setFormat(token, m_commentFormat); break;
			case Token::String: setFormat(token, m_stringFormat); break;
			case Token::Integer: setFormat(token, m_integerFormat); break;
			case Token::AtSymbol: setFormat(token, m_annotationNameFormat); break;
			case Token::Keyword_Enum:
			case Token::Keyword_Struct:
			case Token::Keyword_Using:
				setFormat(token, m_keywordFormat);
				break;
			}
		}
		for (const Document::Diagnostic &diagnostic : data->diagnostics) {
			for (int i = diagnostic.start; i <= diagnostic.end; ++i) {
				setFormat(i - blockStart, 1, makeErrorFormat(format(i)));
			}
		}
		for (const Document::TypeDefinition &def : data->definitions) {
			switch (def.type) {
			case Document::TypeDefinition::Struct:
			case Document::TypeDefinition::Enum:
			case Document::TypeDefinition::Alias:
				setFormat(def.start - currentBlock().position(), def.end - def.start, m_typeFormat);
				break;
			case Document::TypeDefinition::Attribute:
			case Document::TypeDefinition::Entry:
				setFormat(def.start - currentBlock().position(), def.end - def.start, m_attributeNameFormat);
				break;
			}

		}
		for (const Document::TypeUsage &def : data->usages) {
			setFormat(def.start - currentBlock().position(), def.end - def.start, m_typeFormat);
		}
		data->state = lexer.state();
		setCurrentBlockUserData(data);
	}

	using QSyntaxHighlighter::setFormat;
	void setFormat(const QRegularExpressionMatch &match, const QTextCharFormat &format)
	{
		setFormat(match.capturedStart(), match.capturedLength(), format);
	}
	void setFormat(const Token &token, const QTextCharFormat &format)
	{
		setFormat(token.offset - currentBlock().position(), token.length, format);
	}
};

Document::Document(QObject *parent)
	: QTextDocument(parent)
{
	setDocumentLayout(new QPlainTextDocumentLayout(this));
	new SyntaxHighlighter(this);
}

void Document::setFileName(const QString &filename)
{
	m_fileName = filename;
	emit nameChanged(name());
	emit fileNameChanged(m_fileName);
}
QString Document::name() const
{
	return QFileInfo(m_fileName).fileName();
}

void Document::save()
{
	m_savedRevision = revision();
	setModified(false);
}

void Document::setPlainText(const QString &text)
{
	QTextDocument::setPlainText(text);
	save();
	emit diagnosticsReset();
}

QVector<Document::Diagnostic> Document::diagnosticsFor(const QTextCursor &cursor) const
{
	QVector<Diagnostic> out;
	if (cursor.isNull()) {
		for (auto block = begin(); block != end(); block = block.next()) {
			out += UserData::getForBlock(block)->diagnostics;
		}
	} else {
		const QVector<Diagnostic> tmp = UserData::getForBlock(cursor.block())->diagnostics;
		for (const Diagnostic &candidate : tmp) {
			if (candidate.start <= cursor.position() && candidate.end >= cursor.position()) {
				out.append(candidate);
			}
		}
	}
	return out;
}

QVector<Document::Diagnostic> Document::diagnosticsFor(const QTextBlock &block) const
{
	return UserData::getForBlock(block)->diagnostics;
}

Document::TypeDefinition Document::findDefinitionFor(const QString &name) const
{
	for (auto block = begin(); block != end(); block = block.next()) {
		const std::shared_ptr<UserData> data = UserData::getForBlock(block);
		for (const TypeDefinition &def : data->definitions) {
			if (def.name == name) {
				return def;
			}
		}
	}
	return {};
}

QVector<Document::TypeUsage> Document::findUsagesFor(const QString &name) const
{
	QVector<TypeUsage> out;
	for (auto block = begin(); block != end(); block = block.next()) {
		const std::shared_ptr<UserData> data = UserData::getForBlock(block);
		for (const TypeUsage &usage : data->usages) {
			if (usage.name == name) {
				out.append(usage);
			}
		}
	}
	return out;
}

QString Document::Diagnostic::icon() const
{
	switch (type) {
	case Error: return "dialog-error";
	case Warning: return "dialog-warning";
	case Notice: return "dialog-information";
	}
}

bool Document::Diagnostic::operator==(const Document::Diagnostic &other) const
{
	return type == other.type && start == other.start && end == other.end && string == other.string;
}
