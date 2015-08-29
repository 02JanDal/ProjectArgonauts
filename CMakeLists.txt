cmake_minimum_required(VERSION 3.0)

project(ProjectArgonauts VERSION 0.0.1)
enable_testing()

find_package(Boost 1.58 REQUIRED COMPONENTS filesystem system iostreams) # header only: format algorithm

set(CMAKE_CXX_FLAGS "-std=c++14 -Wextra -Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-local-type-template-args -Wno-c++98-compat-pedantic -Wno-padded")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS} /usr/include/c++/5.2.0/)

add_library(argonauts_util STATIC
	util/ArgonautsException.h
	util/Error.h
	util/Error.cpp

	util/SelfContainerIterator.h
	util/Variant.h

	util/StringUtil.h
	util/StringUtil.cpp
	util/Util.h
	util/Util.cpp
	util/TermUtil.h
	util/TermUtil.cpp
	util/OsUtil.h
	util/OsUtil.cpp
	util/FSUtil.h
	util/FSUtil.cpp

	util/CmdParser.h
	util/CmdParser.cpp
	util/Process.h
	util/Process.cpp

	util/SaxSink.h
	util/SaxSink.cpp

	util/json/Json.h
	util/json/Json.cpp
	util/json/JsonValue.h
	util/json/JsonValue.cpp
	util/json/JsonSaxReader.h
	util/json/JsonSaxReader.cpp
	util/json/JsonSaxWriter.h
	util/json/JsonSaxWriter.cpp
	util/json/JsonReference.h
	util/json/JsonReference.cpp
	util/json/JsonPointer.h
	util/json/JsonPointer.cpp
	util/json/JsonSchema.h
	util/json/JsonSchema.cpp
	util/json/JsonSchemaResolver.h
	util/json/JsonSchemaResolver.cpp
)
target_compile_options(argonauts_util PRIVATE -fPIC)
target_include_directories(argonauts_util PRIVATE util)
target_link_libraries(argonauts_util PUBLIC ${Boost_LIBRARIES})

add_library(libargonauts SHARED
	runtime/Argonauts.h
	runtime/Argonauts.cpp
	runtime/ArgonautsBase.h
	runtime/ArgonautsBase.cpp
	runtime/Parser.h
	runtime/Parser.cpp
	runtime/Serializer.h
	runtime/Serializer.cpp
)
target_link_libraries(libargonauts PUBLIC argonauts_util)
target_include_directories(libargonauts PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/runtime>
	$<INSTALL_INTERFACE:include>)
set_target_properties(libargonauts PROPERTIES OUTPUT_NAME argonauts VERSION ${PROJECT_VERSION})

add_subdirectory(embeddedcpptemplate)
add_subdirectory(tool)
add_subdirectory(example)
add_subdirectory(test)

install(TARGETS libargonauts argonauts_util
	EXPORT Argonauts
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
)
install(EXPORT Argonauts DESTINATION cmake)
install(FILES runtime/ArgonautsFunctions.cmake DESTINATION cmake)
install(DIRECTORY runtime/ DESTINATION include FILES_MATCHING PATTERN *.h)
