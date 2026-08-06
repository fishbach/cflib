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
FetchContent_MakeAvailable(doctest_src)
include(${doctest_src_SOURCE_DIR}/scripts/cmake/doctest.cmake)
