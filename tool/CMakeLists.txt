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

compile_ect(TEMPLATE_SRC ${CMAKE_CURRENT_BINARY_DIR}/templates
	compilers/cpp/templates/EnumHeader.ect
	compilers/cpp/templates/EnumSource.ect
	compilers/cpp/templates/StructHeader.ect
	compilers/cpp/templates/StructSource.ect
	compilers/doc/templates/Doc.ect
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tool_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/tool_config.h)

add_executable(argonauts
	${TEMPLATE_SRC}

	main.cpp
	Lexer.h
	Lexer.cpp
	Parser.h
	Parser.cpp
	Resolver.h
	Resolver.cpp
	DataTypes.h
	DataTypes.cpp
	Compiler.h
	Compiler.cpp
	Importer.h
	Importer.cpp
	VerifyCommand.h
	VerifyCommand.cpp

	compilers/AbstractCompiler.h
	compilers/AbstractCompiler.cpp
	compilers/CppCompiler.h
	compilers/CppCompiler.cpp
	compilers/JsonSchemaCompiler.h
	compilers/JsonSchemaCompiler.cpp
	compilers/RubyCompiler.h
	compilers/RubyCompiler.cpp
	compilers/DocCompiler.h
	compilers/DocCompiler.cpp

	compilers/cpp/TypeProviders.h
	compilers/cpp/TypeProviders.cpp
)
target_link_libraries(argonauts PRIVATE argonauts_util ${Boost_LIBRARIES})
target_include_directories(argonauts PRIVATE ${Boost_INCLUDE_DIRS})

install(TARGETS argonauts
	EXPORT Argonauts
	RUNTIME DESTINATION bin
)
