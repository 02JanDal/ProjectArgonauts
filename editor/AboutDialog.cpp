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

#include "AboutDialog.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>

#include "tool_config.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	setWindowIcon(QIcon::fromTheme("help-about"));
	setWindowTitle(tr("About"));
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(new QLabel(tr("<h1><center>Argonauts Editor <small>%1</small><center></h1><p>This program is part of ProjectArgonauts, a code and documentation generator for an IDL.</p>").arg(ARGONAUTS_VERSION)));
	layout->addWidget(new QLabel(tr("<h2><center>Credits and Acknowledgements<center></h2><ul>\
<li>Qt 5 (%1) - The library used for this editor<li>\
<li>boost (%2) - Used by the command line tool</li>\
<li><a href=\"https://github.com/02JanDal\">Jan Dalheimer</a> - Owner and maintainer</li>\
</ul>").arg(QT_VERSION_STR, ARGONAUTS_BOOST_VERSION)));
	layout->addWidget(new QLabel("<h2><center>License<center></h2>\
<p><center>Copyright 2015 Jan Dalheimer <jan@dalheimer.de><br/>\
<br/>\
Licensed under the Apache License, Version 2.0 (the \"License\");<br/>\
you may not use this file except in compliance with the License.<br/>\
You may obtain a copy of the License at<br/>\
<br/>\
	http://www.apache.org/licenses/LICENSE-2.0<br/>\
<br/>\
Unless required by applicable law or agreed to in writing, software<br/>\
distributed under the License is distributed on an \"AS IS\" BASIS,<br/>\
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br/>\
See the License for the specific language governing permissions and<br/>\
limitations under the License.</center></p>"));
}
