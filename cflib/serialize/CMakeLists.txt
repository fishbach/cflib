# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_serialize
    PUBLIC
        cflib_util
    DIRS
        generate
        impl
)

add_subdirectory(asn1dump)
add_subdirectory(ser)
if(BUILD_TESTS)
    add_subdirectory(serialize_test)
endif()
