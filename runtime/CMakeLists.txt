add_library(libargonauts SHARED
	Argonauts.h
	Argonauts.cpp
	Parser.h
	Parser.cpp
	Serializer.h
	Serializer.cpp
)
target_link_libraries(libargonauts PUBLIC argonauts_util)
target_include_directories(libargonauts PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/runtime>
	$<INSTALL_INTERFACE:include/runtime>)
target_compile_options(libargonauts PRIVATE "-Werror")
set_target_properties(libargonauts PROPERTIES OUTPUT_NAME argonauts VERSION ${PROJECT_VERSION})

install(TARGETS libargonauts EXPORT Argonauts
	LIBRARY DESTINATION lib
	INCLUDES DESTINATION include
)
