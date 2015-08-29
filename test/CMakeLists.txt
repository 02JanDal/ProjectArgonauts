find_package(Wget)

if(WGET_FOUND)
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Catch)
	add_custom_command(OUTPUT Catch/catch.hpp
		COMMAND ${WGET_EXECUTABLE} -O Catch/catch.hpp https://raw.githubusercontent.com/philsquared/Catch/master/single_include/catch.hpp
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	add_custom_target(Catch DEPENDS Catch/catch.hpp)

	add_executable(tests main.cpp
		json/tst_JsonSax.cpp
		json/tst_JsonValue.cpp
		json/tst_JsonPointer.cpp
		json/tst_JsonReference.cpp
		tst_StringUtil.cpp
		tst_CmdParser.cpp
	)
	target_link_libraries(tests argonauts_util)
	target_include_directories(tests PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/Catch ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/util)
	target_compile_options(tests PRIVATE "-Wno-unreachable-code")
	add_dependencies(tests Catch)
	add_test(NAME tests COMMAND tests)
else()
	message(WARNING "Could not find the wget executable. Tests will not be built nor run.")
endif()
