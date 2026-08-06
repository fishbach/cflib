# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

# Botan
if(NOT ONLY_GENERATORS)
    find_package(
        Botan 3.12.0
        REQUIRED
    )
endif()

# PostgreSQL
if(ENABLE_PSQL)
    find_package(PostgreSQL REQUIRED)
endif()

# ZLIB
find_package(ZLIB REQUIRED)

# Threads
find_package(Threads REQUIRED)

# doctest
if(BUILD_TESTS)
    find_package(
        doctest 2.5.3
        REQUIRED
    )
endif()
