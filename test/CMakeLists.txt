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

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Catch)
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Catch/catch.hpp)
	file(DOWNLOAD https://raw.githubusercontent.com/philsquared/Catch/master/single_include/catch.hpp ${CMAKE_CURRENT_BINARY_DIR}/Catch/catch.hpp)
endif()
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/cases.mpac)
	file(DOWNLOAD https://rawgit.com/msgpack/msgpack-c/master/test/cases.mpac ${CMAKE_CURRENT_BINARY_DIR}/cases.mpac)
endif()

add_executable(tests main.cpp
	json/tst_JsonSax.cpp
	json/tst_JsonValue.cpp
	json/tst_JsonPointer.cpp
	json/tst_JsonReference.cpp
	tst_StringUtil.cpp
	tst_CmdParser.cpp
	tst_Variant.cpp
)
target_link_libraries(tests argonauts_util)
target_include_directories(tests PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/Catch ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/util)
target_compile_options(tests PRIVATE "-Wno-unreachable-code -Wno-exit-time-destructors -Wno-string-conversion -Wno-shadow")
add_test(NAME tests COMMAND tests)
