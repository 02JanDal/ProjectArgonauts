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

add_executable(embeddedcpptemplate
	main.cpp
)
target_link_libraries(embeddedcpptemplate PRIVATE argonauts_util ${Boost_LIBRARIES})
target_include_directories(embeddedcpptemplate PRIVATE ${Boost_INCLUDE_DIRS})

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
