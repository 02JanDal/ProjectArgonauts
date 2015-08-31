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

if(ARGONAUTS_FUNCTIONS_CMAKE)
	return()
endif()
set(ARGONAUTS_FUNCTIONS_CMAKE 1)

if(NOT TARGET argonauts)
	find_package(Argonauts REQUIRED)
endif()

include(CMakeParseArguments)
function(process_argonauts_cpp)
	cmake_parse_arguments(PAC "" "OUTVAR;TYPES;OUTDIR;TARGET" "INFILES" ${ARGN})
	if(NOT PAC_TYPES)
		set(PAC_TYPES "stl")
	endif()

	set(finalResult )
	foreach(file ${PAC_INFILES})
		file(STRINGS ${file} types REGEX "[\\w^]*(struct|enum) ([^ <]*)")
		set(resultFiles )
		foreach(type ${types})
			string(REGEX REPLACE "[\\w^]*(struct|enum) ([^ <]*).*" "\\2" type ${type})
			list(APPEND resultFiles ${PAC_OUTDIR}/${type}.arg.h ${PAC_OUTDIR}/${type}.arg.cpp)
		endforeach()

		add_custom_command(OUTPUT ${resultFiles} VERBATIM
			COMMAND argonauts compile cpp --output ${PAC_OUTDIR} --types ${PAC_TYPES} ${file}
			COMMENT "Generating from argonauts grammar ${file}..."
			MAIN_DEPENDENCY ${file}
			DEPENDS argonauts
		)

		list(APPEND finalResult ${resultFiles})
	endforeach()

	if(PAC_TARGET)
		add_custom_target(${PAC_TARGET} DEPENDS ${finalResult})
	endif()

	set(${PAC_OUTVAR} ${finalResult} PARENT_SCOPE)
endfunction()

function(process_argonauts_doc)
	cmake_parse_arguments(PAD "" "OUTDIR;TARGET" "INFILES" ${ARGN})

	add_custom_command(OUTPUT ${PAD_OUTDIR}/index.html VERBATIM
		COMMAND argonauts compile doc --output ${PAD_OUTDIR} ${PAD_INFILES}
		COMMENT "Generating documentation for argonauts grammar..."
		DEPENDS argonauts
	)

	if(PAD_TARGET)
		add_custom_target(${PAD_TARGET} DEPENDS ${PAD_OUTDIR}/index.html)
	endif()
endfunction()
