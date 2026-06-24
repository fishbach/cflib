# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_util
    PUBLIC cflib_base
    PRIVATE ZLIB::ZLIB Threads::Threads
    DIRS impl
    OTHER_FILES
        impl/generate_templates.pl
)

add_subdirectory(bin2src)
add_subdirectory(gitversion)
add_subdirectory(jscombiner)
if(BUILD_TESTS)
    add_subdirectory(util_test)
endif()
