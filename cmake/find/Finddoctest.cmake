# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(FetchContent)
FetchContent_Declare(
  doctest_src
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v${doctest_FIND_VERSION}
)

# suppress: CMake Deprecation Warning at build/_deps/doctest_src-src/CMakeLists.txt:1 (cmake_minimum_required):
set(OLD_CMAKE_WARN_DEPRECATED ${CMAKE_WARN_DEPRECATED})
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(doctest_src)

set(CMAKE_WARN_DEPRECATED ${OLD_CMAKE_WARN_DEPRECATED} CACHE BOOL "" FORCE)

list(APPEND CMAKE_MODULE_PATH "${doctest_src_SOURCE_DIR}/scripts/cmake")
