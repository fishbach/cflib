# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_util
	PUBLIC Qt5::Core
	PRIVATE ZLIB::ZLIB
	DIRS impl
)

add_subdirectory(gitversion)
add_subdirectory(jscombiner)
add_subdirectory(util_test)
