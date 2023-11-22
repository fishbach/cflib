# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_crypt
    PUBLIC cflib_util
    PRIVATE cflib_botan
    DIRS impl
)

add_subdirectory(crypt_test)
add_subdirectory(pwhash)
