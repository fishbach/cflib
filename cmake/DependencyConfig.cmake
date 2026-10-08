# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

if(USE_FETCHCONTENT)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/find")
endif()

# Botan
if(NOT ONLY_GENERATORS)
    find_package(
        Botan 3.10.0
        REQUIRED
    )
    if(NOT USE_FETCHCONTENT)
        add_library(cflib_botan ALIAS botan::botan)
    endif()
endif()

# PostgreSQL
if(ENABLE_PSQL)
    find_package(PostgreSQL REQUIRED)
endif()

# ZLIB
find_package(ZLIB REQUIRED)

# Threads
find_package(Threads REQUIRED)

# SQLite
if(ENABLE_SQLITE)
    find_package(
        SQLite3 3.46.1
        REQUIRED
    )
    if(NOT USE_FETCHCONTENT)
        add_library(cflib_sqlite ALIAS SQLite::SQLite3)
    endif()
endif()

# doctest
if(BUILD_TESTS)
    find_package(
        doctest 2.4.12
        REQUIRED
    )
    include(doctest)
endif()
