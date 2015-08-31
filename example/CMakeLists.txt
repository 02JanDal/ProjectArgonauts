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

add_custom_target(dummy_target SOURCES grammar.arg)

include(../runtime/ArgonautsFunctions.cmake)

process_argonauts_cpp(OUTVAR generated OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/grammar INFILES ${CMAKE_CURRENT_SOURCE_DIR}/grammar.arg TARGET argonauts_example_grammar)
process_argonauts_doc(OUTDIR ${CMAKE_CURRENT_BINARY_DIR} INFILES ${CMAKE_CURRENT_SOURCE_DIR}/grammar.arg TARGET argonauts_example_grammar_doc)

add_executable(argonauts_example main.cpp ${generated})
target_link_libraries(argonauts_example libargonauts)
add_dependencies(argonauts_example argonauts_example_grammar argonauts_example_grammar_doc)
