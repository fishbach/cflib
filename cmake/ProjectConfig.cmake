# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

# look once for config files
if(NOT cf_config_file_path)
    set(cf_config_file_list)
    set(cf_config_file_path "${PROJECT_SOURCE_DIR}")
    while(NOT cf_config_file_path STREQUAL "/")
        if(EXISTS "${cf_config_file_path}/config.cmake")
            list(APPEND cf_config_file_list "${cf_config_file_path}/config.cmake")
        endif()
        get_filename_component(cf_config_file_path "${cf_config_file_path}" DIRECTORY)
    endwhile()
    list(REVERSE cf_config_file_list)
    foreach(config ${cf_config_file_list})
        message(STATUS "Importing \"${config}\"")
        include("${config}")
    endforeach()
    unset(cf_config_file_list)
endif()

# generator binaries and support paths, used by cf_lib/cf_app/cf_test
# (when cflib is used from installed packages, cflibConfig.cmake provides these instead)
set(CFLIB_GEN_SER         ser)
set(CFLIB_GEN_APIEXPORTER apiexporter)
set(CFLIB_GEN_BIN2SRC     bin2src)
set(CFLIB_PCH_HEADER      "${CMAKE_CURRENT_LIST_DIR}/../cflib/base.h")
set(CFLIB_TESTMAIN_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../cflib/util/testmain/main.cpp")

# config options
option(USE_FETCHCONTENT "use FetchContent for required libs" ON)
option(ENABLE_CCACHE    "enable ccache"                      ON)
option(ENABLE_PCH       "enable precompiled headers (PCH)"   ON)
option(ENABLE_PSQL      "enable PostgreSQL"                  OFF)
option(ENABLE_SQLITE    "enable SQLite"                      OFF)
option(ONLY_GENERATORS  "only generators are build"          OFF)
set(GENERATORS_BIN_DIR  "" CACHE PATH "Use generators from given directory. Useful for cross compilation.")

# C++20
set(CMAKE_CXX_STANDARD          20 )
set(CMAKE_CXX_STANDARD_REQUIRED ON )
set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES  OFF)

# ccache
if(ENABLE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
        message(STATUS "Using ccache")
        set(CMAKE_CXX_COMPILER_LAUNCHER CCACHE_SLOPPINESS=include_file_ctime,include_file_mtime,pch_defines,time_macros ccache)
    endif()
endif()

# PCH
if(ENABLE_PCH)
    message(STATUS "Using PCH")
    if(CMAKE_POSITION_INDEPENDENT_CODE AND NOT MSVC)
        add_compile_options(-fPIE)
    endif()
endif()

# PostgreSQL
if(ENABLE_PSQL)
    message(STATUS "Building PostgreSQL parts")
endif()

# SQLite
if(ENABLE_SQLITE)
    message(STATUS "Building SQLite parts")
endif()

# Warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wcast-align
        # -Wconversion
        -Wdouble-promotion
        -Wformat=2
        # -Wnon-virtual-dtor
        -Wnull-dereference
        # -Woverloaded-virtual
        # -Wshadow
        # -Wsign-conversion
        -Wunused
    )
endif()

# enable tests
if(BUILD_TESTS)
    enable_testing()
endif()

# cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
include(Build)
include(Util)

# If only this file is included, add cflib as a whole.
if(NOT TARGET cflib_base AND NOT "${CMAKE_CURRENT_LIST_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}/cmake")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/..)
elseif(TARGET cflib_base)
    get_target_property(_cflib_base_imported cflib_base IMPORTED)
    if(_cflib_base_imported)
        message(FATAL_ERROR
            "cflib: target 'cflib_base' is an IMPORTED target - find_package(cflib) was already called.\n"
            "Do not mix installed package usage (find_package(cflib)) with source usage\n"
            "(include(../3rd_party/cflib/cmake/ProjectConfig.cmake) / add_subdirectory). Use one or the other."
        )
    endif()
endif()
