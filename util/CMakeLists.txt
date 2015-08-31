add_library(argonauts_util STATIC
	ArgonautsException.h
	Error.h
	Error.cpp

	SelfContainerIterator.h
	Variant.h

	StringUtil.h
	StringUtil.cpp
	Util.h
	Util.cpp
	TermUtil.h
	TermUtil.cpp
	OsUtil.h
	FSUtil.h
	FSUtil.cpp

	CmdParser.h
	CmdParser.cpp

	SaxSink.h
	SaxSink.cpp

	json/Json.h
	json/Json.cpp
	json/JsonValue.h
	json/JsonValue.cpp
	json/JsonSaxReader.h
	json/JsonSaxReader.cpp
	json/JsonSaxWriter.h
	json/JsonSaxWriter.cpp
	json/JsonReference.h
	json/JsonReference.cpp
	json/JsonPointer.h
	json/JsonPointer.cpp
	json/JsonSchema.h
	json/JsonSchema.cpp
	json/JsonSchemaResolver.h
	json/JsonSchemaResolver.cpp
)
target_compile_options(argonauts_util PRIVATE -fPIC)
target_include_directories(argonauts_util PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/util>
	$<INSTALL_INTERFACE:include/util>)
#target_compile_options(argonauts_util PRIVATE "-Werror")

install(TARGETS argonauts_util EXPORT Argonauts
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
)
