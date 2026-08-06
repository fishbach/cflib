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

if(BUILD_TESTS)
    add_library(cflib_testmain OBJECT testmain/main.cpp)
    target_link_libraries(cflib_testmain PUBLIC doctest::doctest)
endif()

add_subdirectory(bin2src)
add_subdirectory(jscombiner)
if(BUILD_TESTS)
    add_subdirectory(util_test)
endif()
