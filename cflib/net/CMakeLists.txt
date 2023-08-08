# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_net
	PUBLIC cflib_crypt
	PUBLIC cflib_serialize
	DIRS impl
)

add_subdirectory(net_test)
