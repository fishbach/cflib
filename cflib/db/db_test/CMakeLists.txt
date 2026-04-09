# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_test(db_test cflib_db
    RESOURCES schema.sql
)

if(ENABLE_PSQL)
    cf_find_sources(PSQL_SOURCES psql)
    target_sources(db_test PRIVATE ${PSQL_SOURCES})
endif()

if(ENABLE_SQLITE)
    cf_find_sources(SQLITE_SOURCES sqlite)
    target_sources(db_test PRIVATE ${SQLITE_SOURCES})
endif()
