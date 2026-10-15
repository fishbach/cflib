# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.28)

project(
    cflib
    VERSION 1.0.0
    LANGUAGES C CXX
)

option(CFLIB_BUILD_TESTS    "activate build for tests"      ON                     )
option(CFLIB_BUILD_BENCH    "activate build for benchmarks" ${PROJECT_IS_TOP_LEVEL})
option(CFLIB_BUILD_EXAMPLES "activate build for examples"   ${PROJECT_IS_TOP_LEVEL})

if(NOT COMMAND cf_lib)
    include(cmake/ProjectConfig.cmake)
endif()
include(DependencyConfig)

cf_add_other(cflib_files
    .gitignore
    COPYING
    MODULES
    README.md
    js/*
)

add_subdirectory(cflib)
if(CFLIB_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
