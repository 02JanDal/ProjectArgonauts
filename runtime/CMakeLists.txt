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
	$<INSTALL_INTERFACE:include/runtime>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
target_compile_options(libargonauts PRIVATE "-Werror")
set_target_properties(libargonauts PROPERTIES
	OUTPUT_NAME argonauts
	VERSION ${PROJECT_VERSION}
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN 1
)

include(GenerateExportHeader)
generate_export_header(libargonauts)

install(TARGETS libargonauts EXPORT Argonauts
	LIBRARY DESTINATION lib
	INCLUDES DESTINATION include
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libargonauts_export.h DESTINATION include/runtime)
