add_custom_target(dummy_target SOURCES grammar.arg)

include(../runtime/ArgonautsFunctions.cmake)

process_argonauts_cpp(OUTVAR generated OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/grammar INFILES ${CMAKE_CURRENT_SOURCE_DIR}/grammar.arg TARGET argonauts_example_grammar)
process_argonauts_doc(OUTDIR ${CMAKE_CURRENT_BINARY_DIR} INFILES ${CMAKE_CURRENT_SOURCE_DIR}/grammar.arg TARGET argonauts_example_grammar_doc)

add_executable(argonauts_example main.cpp ${generated})
target_link_libraries(argonauts_example libargonauts)
add_dependencies(argonauts_example argonauts_example_grammar argonauts_example_grammar_doc)
