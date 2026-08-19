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

option(BUILD_TESTS    "activate build for tests"      ON)
option(BUILD_EXAMPLES "activate build for examples"   ON)
option(BUILD_BENCH    "activate build for benchmarks" ON)

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
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if(BUILD_BENCH)
    add_subdirectory(bench)
endif()
