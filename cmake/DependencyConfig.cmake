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
    find_package(Botan 3.13.0 REQUIRED)
    if(USE_FETCHCONTENT)
        # cflib_botan is a real target built from source,
        # it gets installed alongside cflib
        set(CFLIB_BOTAN_BUNDLED TRUE)
    else()
        set(CFLIB_BOTAN_BUNDLED FALSE)
        add_library(cflib_botan ALIAS botan::botan)
    endif()
else()
    set(CFLIB_BOTAN_BUNDLED FALSE)
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
    if(USE_FETCHCONTENT)
        find_package(SQLite3 3.53.4 REQUIRED)
        set(CFLIB_SQLITE_VERSION 3.53.4)
        # cflib_sqlite is a real target built from source,
        # it gets installed alongside cflib
        set(CFLIB_SQLITE_BUNDLED TRUE)
    else()
        find_package(SQLite3 3.46.1 REQUIRED)
        set(CFLIB_SQLITE_VERSION 3.46.1)
        set(CFLIB_SQLITE_BUNDLED FALSE)
        add_library(cflib_sqlite ALIAS SQLite::SQLite3)
    endif()
else()
    set(CFLIB_SQLITE_BUNDLED FALSE)
endif()

# doctest
if(BUILD_TESTS)
    if(USE_FETCHCONTENT)
        find_package(doctest 2.5.3 REQUIRED)
    else()
        find_package(doctest 2.4.12 REQUIRED)
    endif()
    include(doctest)
endif()
