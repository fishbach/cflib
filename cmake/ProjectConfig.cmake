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
	cmake_parse_arguments(ARG "ENABLE_MOC;ENABLE_SER" "" "PUBLIC;PRIVATE;DIRS;OTHER_FILES" ${ARGN})

	cf_find_sources(sources . ${ARG_DIRS} OTHER_FILES ${ARG_OTHER_FILES})
	add_library(${lib} ${sources})
	cf_configure_target(${lib} ${ARG_ENABLE_MOC} ${ARG_ENABLE_SER})
	target_link_libraries(${lib} PUBLIC ${ARG_PUBLIC} PRIVATE ${ARG_PRIVATE})
endfunction()

# application
function(cf_app app)
	cmake_parse_arguments(ARG "ENABLE_MOC;ENABLE_SER" "" "DIRS;OTHER_FILES" ${ARGN})

	cf_find_sources(sources . ${ARG_DIRS} OTHER_FILES ${ARG_OTHER_FILES})
	add_executable(${app} ${sources})
	cf_configure_target(${app} ${ARG_ENABLE_MOC} ${ARG_ENABLE_SER})
	target_link_libraries(${app} PRIVATE ${ARG_UNPARSED_ARGUMENTS})
endfunction()

# tests
enable_testing()

function(cf_tests test lib)
	cmake_parse_arguments(ARG "ENABLE_SER" "" "DIRS" ${ARGN})

	cf_find_sources(sources . ${ARG_DIRS})
	add_executable(${test} ${sources})
	cf_configure_target(${test} TRUE ${ARG_ENABLE_SER})
	target_link_libraries(${test} PRIVATE ${lib} Qt5::Test)
	add_test(NAME ${test} COMMAND ${test})
endfunction()
