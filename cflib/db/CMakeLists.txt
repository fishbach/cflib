# Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

if(ENABLE_PSQL)
    set(PSQL_CONFIG
        PRIVATE PostgreSQL::PostgreSQL
        DIRS psql
    )
endif()

cf_lib(cflib_db
    PUBLIC cflib_util
    PUBLIC Qt5::Sql
    ${PSQL_CONFIG}
    DIRS impl
    ENABLE_MOC
)

if(ENABLE_PSQL)
    add_subdirectory(db_test)
    add_subdirectory(migrationmoped)
endif()
