add_executable(embeddedcpptemplate
	main.cpp
)
target_link_libraries(embeddedcpptemplate PRIVATE argonauts_util)

function(compile_ect SOURCES_OUT DIR)
	set(result )
	foreach(file ${ARGN})
		get_filename_component(dir ${file} DIRECTORY)
		get_filename_component(file ${file} NAME)
		add_custom_command(OUTPUT ${DIR}/${file}.h ${DIR}/${file}.cpp
			COMMAND embeddedcpptemplate -o ${DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${file}
			COMMENT "Processing template ${file}..." VERBATIM
			MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${file}
			DEPENDS embeddedcpptemplate
		)
		list(APPEND result ${DIR}/${file}.h ${DIR}/${file}.cpp)
	endforeach()
	set(${SOURCES_OUT} ${result} PARENT_SCOPE)
endfunction()
