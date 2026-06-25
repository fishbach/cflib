# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

function(cf_find_sources var)
    cmake_parse_arguments(ARG "" "" "OTHER_FILES" ${ARGN})

    set(${var})
    foreach(dir ${ARG_UNPARSED_ARGUMENTS})
        if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
            list(APPEND ${var} "${dir}")
            continue()
        endif()
        file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${dir}/*.h"
            "${dir}/*.cpp"
            "${dir}/*.qrc"
            "${dir}/*.ui"
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
        file(GLOB_RECURSE files FOLLOW_SYMLINKS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_OTHER_FILES})
        list(APPEND ${var} ${files})
        set_source_files_properties(${files} PROPERTIES HEADER_FILE_ONLY ON)
    endif()

    set(${var} ${${var}} PARENT_SCOPE)
endfunction()

function(cf_add_other target)
    file(GLOB_RECURSE files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARGN})
    add_custom_target(${target} SOURCES ${files})
endfunction()

function(cf_set_version_defines file)
    set(VERSION_DEFINES
        PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PROJECT_VERSION_SUFFIX=\"${PROJECT_VERSION_SUFFIX}\"
    )
    set_source_files_properties(${file} PROPERTIES COMPILE_DEFINITIONS "${VERSION_DEFINES}")
endfunction()

function(cf_git_version var)
    if(NOT "$ENV{CI_COMMIT_SHA}" STREQUAL "")
        set(GIT_HASH $ENV{CI_COMMIT_SHA})
    else()
        execute_process(
            COMMAND git rev-parse HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()
    set(${var} ${GIT_HASH} PARENT_SCOPE)
endfunction()
