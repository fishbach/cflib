# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

function(cf_find_sources var)
	cmake_parse_arguments(ARG "" "" "OTHER_FILES" ${ARGN})

	foreach(dir ${ARG_UNPARSED_ARGUMENTS})
		file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			${dir}/*.h
			${dir}/*.cpp
		)
		list(APPEND ${var} ${files})

		if(APPLE)
			file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${dir}/*.mm)
			set_source_files_properties(${files} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
			list(APPEND ${var} ${files})
		endif()
	endforeach()

	# other files
	if(ARG_OTHER_FILES)
		file(GLOB_RECURSE files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_OTHER_FILES})
		list(APPEND ${var} ${files})
		set_source_files_properties(${files} PROPERTIES HEADER_FILE_ONLY ON)
	endif()

	set(${var} ${${var}} PARENT_SCOPE)
endfunction()

function(cf_add_other target)
	file(GLOB_RECURSE files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARGN})
	add_custom_target(${target} SOURCES ${files})
endfunction()
