# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

set(path "${PROJECT_SOURCE_DIR}")
while(NOT path STREQUAL "/")
	if(EXISTS "${path}/config.cmake")
		message(STATUS "Importing \"${path}/config.cmake\"")
		include("${path}/config.cmake")
	endif()
	get_filename_component(path "${path}" DIRECTORY)
endwhile()
unset(path)
