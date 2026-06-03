# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_crypt
    PUBLIC cflib_util
    PRIVATE cflib_botan
    DIRS impl
    PCH pch.h
    ENABLE_EXCEPTIONS
)

if(BUILD_TESTS)
    add_subdirectory(crypt_test)
endif()
add_subdirectory(pwhash)
