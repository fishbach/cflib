# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

function(cf_find_sources var)
	foreach(dir ${ARGN})
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
	set(${var} ${${var}} PARENT_SCOPE)
endfunction()
