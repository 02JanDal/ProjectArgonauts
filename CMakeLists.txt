cmake_minimum_required(VERSION 3.0)

project(ProjectArgonauts VERSION 0.0.1)
enable_testing()

set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(/usr/include/c++/5.1.0/)

add_library(argonauts_util STATIC
	util/SelfContainerIterator.h
	util/ArgonautsException.h
	util/StringUtil.h
	util/StringUtil.cpp
	util/Util.h
	util/Util.cpp
	util/CmdParser.h
	util/CmdParser.cpp
	util/TermUtil.h
	util/TermUtil.cpp
	util/OsUtil.h
	util/OsUtil.cpp
	util/JsonSax.h
	util/JsonSax.cpp
)

add_executable(argonauts
	tool/main.cpp
	tool/Lexer.h
	tool/Lexer.cpp
	tool/Parser.h
	tool/Parser.cpp
	tool/DataTypes.h
	tool/DataTypes.cpp
	tool/Compiler.h
	tool/Compiler.cpp
	tool/Importer.h
	tool/Importer.cpp
	tool/DocGenerator.h
	tool/DocGenerator.cpp
)
target_link_libraries(argonauts argonauts_util)
add_executable(argonauts-compiler-cpp compilers/cpp/main.cpp)
target_link_libraries(argonauts-compiler-cpp argonauts_util)
add_executable(argonauts-compiler-jsonschema compilers/jsonschema/main.cpp)
target_link_libraries(argonauts-compiler-jsonschema argonauts_util)
add_executable(argonauts-compiler-ruby compilers/ruby/main.cpp)
target_link_libraries(argonauts-compiler-ruby argonauts_util)

add_custom_target(dummy_target SOURCES grammar.arg)

configure_file(${CMAKE_SOURCE_DIR}/tool/tool_config.h.in ${CMAKE_BINARY_DIR}/tool_config.h)

add_subdirectory(test)
