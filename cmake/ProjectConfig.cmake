# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(CompileConfig)
include(Util)

# library
function(cf_lib lib)
	cmake_parse_arguments(ARG "ENABLE_MOC" "" "PUBLIC;PRIVATE;DIRS" ${ARGN})

	cf_find_sources(sources . ${ARG_DIRS})
	add_library(${lib} ${sources})
	cf_configure_target(${lib} ${ARG_ENABLE_MOC})
	target_link_libraries(${lib} PUBLIC ${ARG_PUBLIC} PRIVATE ${ARG_PRIVATE})
endfunction()

# application
function(cf_app app)
	cmake_parse_arguments(ARG "ENABLE_MOC" "" "DIRS" ${ARGN})

	cf_find_sources(sources . ${ARG_DIRS})
	add_executable(${app} ${sources})
	cf_configure_target(${app} ${ARG_ENABLE_MOC})
	target_link_libraries(${app} PRIVATE ${ARG_UNPARSED_ARGUMENTS})
endfunction()

# tests
enable_testing()

function(cf_tests lib)
	file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
	foreach(file ${files})
		get_filename_component(exe ${file} NAME_WLE)

		add_executable(${exe} ${file})
		cf_configure_target(${exe} TRUE)
		target_link_libraries(${exe} PRIVATE ${lib} Qt5::Test)

		add_test(NAME ${exe} COMMAND ${exe})
	endforeach()
endfunction()
