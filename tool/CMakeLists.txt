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
target_link_libraries(argonauts PRIVATE argonauts_util)

install(TARGETS argonauts
	EXPORT Argonauts
	RUNTIME DESTINATION bin
)
