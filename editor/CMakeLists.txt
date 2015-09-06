# Copyright 2015 Jan Dalheimer <jan@dalheimer.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

set(CMAKE_AUTOMOC ON)

add_executable(argonauts-editor
	main.cpp

	Document.h
	Document.cpp

	TextEditor.h
	TextEditor.cpp
	GraphicalEditor.h
	GraphicalEditor.cpp

	MainWindow.h
	MainWindow.cpp

	AboutDialog.h
	AboutDialog.cpp
	HelpDialog.h
	HelpDialog.cpp

	DiagnosticsModel.h
	DiagnosticsModel.cpp

	${CMAKE_SOURCE_DIR}/common/Lexer.h
	${CMAKE_SOURCE_DIR}/common/Lexer.cpp
	${CMAKE_SOURCE_DIR}/common/Token.h
	${CMAKE_SOURCE_DIR}/common/Token.cpp
	${CMAKE_SOURCE_DIR}/common/GeneralParser.h
	${CMAKE_SOURCE_DIR}/common/GeneralParser.cpp
)
target_link_libraries(argonauts-editor PRIVATE argonauts_util Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)

install(TARGETS argonauts-editor
	EXPORT Argonauts
	RUNTIME DESTINATION bin
)
