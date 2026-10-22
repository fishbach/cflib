# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

# Threads
find_package(Threads REQUIRED)

# ZLIB
find_package(ZLIB REQUIRED)

if(CFLIB_ONLY_GENERATORS)
    return()
endif()

# PostgreSQL
if(ENABLE_PSQL)
    find_package(PostgreSQL REQUIRED)
endif()

if(USE_FETCHCONTENT)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/find")
endif()

# Botan
find_package(Botan 3.13.0 REQUIRED)
if(NOT USE_FETCHCONTENT)
    add_library(cflib_botan ALIAS botan::botan)
endif()

# SQLite
if(ENABLE_SQLITE)
    if(USE_FETCHCONTENT)
        find_package(SQLite3 3.53.4 REQUIRED)
    else()
        set(CFLIB_SQLITE_SYSTEM_VERSION 3.46.1)
        find_package(SQLite3 ${CFLIB_SQLITE_SYSTEM_VERSION} REQUIRED)
        add_library(cflib_sqlite ALIAS SQLite::SQLite3)
    endif()
endif()

# doctest
set(CFLIB_DOCTEST_SYSTEM_VERSION 2.4.12)
if(CFLIB_BUILD_TESTS)
    if(USE_FETCHCONTENT)
        find_package(doctest 2.5.3 REQUIRED)
    else()
        find_package(doctest ${CFLIB_DOCTEST_SYSTEM_VERSION} REQUIRED)
    endif()
    include(doctest)
endif()
