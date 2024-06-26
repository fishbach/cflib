# Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.16)

project(
    cflib
    VERSION 1.0.0
    LANGUAGES CXX
)

include(cmake/ProjectConfig.cmake)

cf_add_other(cflib_files
    .gitignore
    COPYING
    MODULES
    README.md
    js/*
)

add_subdirectory(cflib)
option(BUILD_EXAMPLES "activate build for examples" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
