# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_serialize
    PUBLIC cflib_util
    DIRS impl
)

add_subdirectory(asn1dump)
add_subdirectory(ser)
add_subdirectory(serialize_test)
