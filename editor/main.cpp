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

#include <QApplication>
#include <QCommandLineParser>

#include "MainWindow.h"
#include "tool_config.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	app.setOrganizationName("02JanDal");
	app.setOrganizationDomain("https://github.com/02JanDal");
	app.setApplicationDisplayName("Argonauts Editor");
	app.setApplicationName("ArgonautsEditor");
	app.setApplicationVersion(ARGONAUTS_VERSION);

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	parser.setApplicationDescription(QObject::tr("Graphical and textual editor for ProjectArgonauts IDL files"));
	parser.addPositionalArgument(QObject::tr("file"), QObject::tr("Name of a file to open"), QObject::tr("FILE"));
	parser.process(app);

	MainWindow window;

	if (!parser.positionalArguments().isEmpty()) {
		window.openDocument(parser.positionalArguments().first());
	}

	window.show();

	return app.exec();
}
