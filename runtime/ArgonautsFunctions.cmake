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
	cmake_parse_arguments(PAC "" "OUTVAR;TYPES;OUTDIR;TARGET;INFILE" "PARSERS;SERIALIZERS;FILETYPES" ${ARGN})
	if(NOT PAC_TYPES)
		set(PAC_TYPES "stl")
	endif()
	set(finalResult )

	if(NOT PAC_INFILE OR "${PAC_INFILE}" STREQUAL "")
		message(AUTHOR_WARNING "Missing required argument: INFILE")
	endif()
	if((NOT "${PAC_PARSERS}" STREQUAL "" OR NOT "${PAC_SERIALIZERS}" STREQUAL "") AND "${PAC_FILETYPES}" STREQUAL "")
		message(AUTHOR_WARNING "Missing required argument: FILETYPES (required because PARSERS or SERIALIZERS was given)")
	endif()

	# General
	file(STRINGS ${PAC_INFILE} types REGEX "[\\w^]*(struct|enum) ([^ <]*)")
	set(resultFiles )
	foreach(type ${types})
		string(REGEX REPLACE "[\\w^]*(struct|enum) ([^ <]*).*" "\\2" type ${type})
		list(APPEND resultFiles ${PAC_OUTDIR}/${type}.arg.h ${PAC_OUTDIR}/${type}.arg.cpp)
	endforeach()
	add_custom_command(OUTPUT ${resultFiles} VERBATIM
		COMMAND argonauts compile cpp --output ${PAC_OUTDIR} --types ${PAC_TYPES} ${PAC_INFILE}
		COMMENT "Generating C++ code from argonauts grammar ${PAC_INFILE}..."
		MAIN_DEPENDENCY ${PAC_INFILE}
		DEPENDS argonauts
	)
	list(APPEND finalResult ${resultFiles})

	set(types ${PAC_PARSERS} ${PAC_SERIALIZERS})
	list(REMOVE_DUPLICATES types)
	foreach(type ${types})
		foreach(filetype ${PAC_FILETYPES})
			if(NOT ";json;msgpack;" MATCHES ";${filetype};")
				message(AUTHOR_WARNING "Unknown argument to FILETYPES: ${filetype}")
			endif()

			set(args )
			set(resultFiles )
			if(";${PAC_PARSERS};" MATCHES ";${type};")
				list(APPEND resultFiles ${PAC_OUTDIR}/${type}.parser.${filetype}.arg.h ${PAC_OUTDIR}/${type}.parser.${filetype}.arg.cpp)
			else()
				list(APPEND args "--no-parser")
			endif()
			if(";${PAC_SERIALIZERS};" MATCHES ";${type};")
				list(APPEND resultFiles ${PAC_OUTDIR}/${type}.serializer.${filetype}.arg.h ${PAC_OUTDIR}/${type}.serializer.${filetype}.arg.cpp)
			else()
				list(APPEND args "--no-serializer")
			endif()
			add_custom_command(OUTPUT ${resultFiles} VERBATIM
				COMMAND argonauts compile cpp-${filetype} --root-type ${type} --output ${PAC_OUTDIR} ${args} --types ${PAC_TYPES} ${PAC_INFILE}
				COMMENT "Generating C++ ${filetype} parser/serializer from argonauts grammar ${PAC_INFILE}..."
				MAIN_DEPENDENCY ${PAC_INFILE}
				DEPENDS argonauts
			)
			list(APPEND finalResult ${resultFiles})
		endforeach()
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
