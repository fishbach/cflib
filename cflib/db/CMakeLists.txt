# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_db
    PUBLIC cflib_util
    PRIVATE PostgreSQL::PostgreSQL
    DIRS psql
)

add_subdirectory(db_test)
add_subdirectory(migrationmoped)
