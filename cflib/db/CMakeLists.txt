# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

if(ENABLE_PSQL OR ENABLE_SQLITE)
    cf_lib(cflib_db PUBLIC cflib_util)
    if(BUILD_TESTS)
        add_subdirectory(db_test)
    endif()
endif()

if(ENABLE_PSQL)
    cf_find_sources(PSQL_SOURCES psql)
    target_sources(cflib_db PRIVATE ${PSQL_SOURCES})
    target_compile_definitions(cflib_db PRIVATE ENABLE_PSQL)
    target_link_libraries(cflib_db PRIVATE PostgreSQL::PostgreSQL)

    add_subdirectory(migrationmoped)
endif()

if(ENABLE_SQLITE)
    cf_find_sources(SQLITE_SOURCES sqlite)
    target_sources(cflib_db PRIVATE ${SQLITE_SOURCES})
    target_compile_definitions(cflib_db PRIVATE ENABLE_SQLITE)
    target_link_libraries(cflib_db PRIVATE cflib_sqlite)
endif()
