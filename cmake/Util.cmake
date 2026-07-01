# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

function(cf_find_sources var)
    cmake_parse_arguments(ARG "" "" "OTHER_FILES" ${ARGN})

    set(rv)
    foreach(dir ${ARG_UNPARSED_ARGUMENTS})
        if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
            list(APPEND rv "${dir}")
            continue()
        endif()
        file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${dir}/*.h"
            "${dir}/*.cpp"
            "${dir}/*.qrc"
            "${dir}/*.ui"
        )
        list(APPEND rv ${files})

        if(APPLE)
            file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${dir}/*.mm)
            set_source_files_properties(${files} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
            list(APPEND rv ${files})
        endif()
    endforeach()

    # other files
    if(ARG_OTHER_FILES)
        file(GLOB_RECURSE files FOLLOW_SYMLINKS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ARG_OTHER_FILES})
        list(APPEND rv ${files})
        set_source_files_properties(${files} PROPERTIES HEADER_FILE_ONLY ON)
    endif()

    set(${var} ${rv} PARENT_SCOPE)
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

function(cf_get_dependent_targets target var)
    set(rv)
    get_target_property(LINKED_LIBS ${target} LINK_LIBRARIES)
    foreach(sub_target IN LISTS LINKED_LIBS)
        if(TARGET ${sub_target})
            cf_get_dependent_targets(${sub_target} sub_rv)
            list(APPEND rv ${sub_target} ${sub_rv})
        endif()
    endforeach()
    list(REMOVE_DUPLICATES rv)
    set(${var} ${rv} PARENT_SCOPE)
endfunction()
