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

cmake_minimum_required(VERSION 3.0)

project(ProjectArgonauts VERSION 0.0.1)
enable_testing()

find_package(Boost 1.55 REQUIRED COMPONENTS filesystem system)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CompilerChecks)
include(CheckCXXSymbolExists)
if(APPLE)
	require_flag_supported("-stdlib=libc++" HAS_LIBCPP_FLAG)
else()
	set_flag_if_supported("-stdlib=libstdc++" HAS_LIBSTDCPP_FLAG)
endif()
set_flag_if_supported("-Wextra" HAS_WEXTRA_FLAG)
set_flag_if_supported("-Wall" HAS_WALL_FLAG)
set_flag_if_supported("-Weverything" HAS_WEVERYTHING_FLAG)
if(HAS_WEVERYTHING_FLAG)
	require_flag_supported("-Wno-c++98-compat -Wno-c++98-compat-local-type-template-args -Wno-c++98-compat-pedantic" HAS_CPP98_WARNING_DISABLE_FLAGS)
	require_flag_supported("-Wno-padded" HAS_WNOPADDED_FLAG)
	require_flag_supported("-Wno-weak-vtables" HAS_WNOWEAKVTABLES_FLAG)
endif()
set_flag_if_supported("-Wno-unknown-pragmas" HAS_NOUNKNOWNPRAGMAS_FLAG)

include(TestBigEndian)
test_big_endian(IS_BIGENDIAN)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

configure_file(${CMAKE_SOURCE_DIR}/tool_config.h.in ${CMAKE_BINARY_DIR}/tool_config.h)

add_subdirectory(embeddedcpptemplate)
add_subdirectory(util)
add_subdirectory(runtime)
add_subdirectory(tool)
add_subdirectory(editor)
add_subdirectory(example)
add_subdirectory(test)

install(EXPORT Argonauts DESTINATION cmake)
install(FILES runtime/ArgonautsFunctions.cmake DESTINATION cmake)
install(DIRECTORY runtime/ DESTINATION include/runtime FILES_MATCHING PATTERN *.h)
install(DIRECTORY util/ DESTINATION include/util FILES_MATCHING PATTERN *.h)
