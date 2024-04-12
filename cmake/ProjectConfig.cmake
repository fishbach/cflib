# Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
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

# config options
option(ENABLE_CCACHE "enable ccache"                    ON)
option(ENABLE_PCH    "enable precompiled headers (PCH)" ON)
option(ENABLE_PSQL   "enable PostgreSQL"                OFF)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ccache
if(ENABLE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
        message(STATUS "Using ccache")
        set(CMAKE_CXX_COMPILER_LAUNCHER CCACHE_SLOPPINESS=include_file_ctime,include_file_mtime,pch_defines,time_macros ccache)
    endif()
endif()

# Qt
find_package(
    Qt5 5.15
    COMPONENTS Core Sql Test
    REQUIRED
)

# add root include dir
include_directories(${PROJECT_SOURCE_DIR})

# enable tests
enable_testing()

# -----------------------------------------------------------------------------

# handle include from outer project
if("${CMAKE_CURRENT_LIST_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}/cmake")
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
    include(Build)
    include(DependencyConfig)
    include(Util)
else()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/..)
endif()
