# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_util
    PUBLIC cflib_base
    PRIVATE ZLIB::ZLIB Threads::Threads
    PCH ${PROJECT_SOURCE_DIR}/pch/util.h
    DIRS impl
    OTHER_FILES
        impl/generate_templates.pl
)
# testmain.cpp is for tests only (linked separately), not part of cflib_util
get_target_property(_util_sources cflib_util SOURCES)
list(REMOVE_ITEM _util_sources testmain.cpp)
set_target_properties(cflib_util PROPERTIES SOURCES "${_util_sources}")

add_subdirectory(gitversion)
add_subdirectory(jscombiner)
add_subdirectory(util_test)
