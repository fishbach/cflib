# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

if(ENABLE_PSQL OR ENABLE_SQLITE)
    cf_lib(cflib_db PUBLIC cflib_util)
    add_subdirectory(db_test)
endif()

if(ENABLE_PSQL)
    cf_find_sources(PSQL_SOURCES psql)
    target_sources(cflib_db PRIVATE ${PSQL_SOURCES})
    target_compile_definitions(cflib_db PRIVATE ENABLE_PSQL)
    target_link_libraries(cflib_db PRIVATE PostgreSQL::PostgreSQL)

    add_subdirectory(migrationmoped)
endif()

if(ENABLE_SQLITE)
    include(FetchContent)
    FetchContent_Declare(
        sqlite3_src
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        URL https://sqlite.org/2026/sqlite-amalgamation-3510300.zip
    )
    FetchContent_MakeAvailable(sqlite3_src)

    target_sources(cflib_db PRIVATE ${sqlite3_src_SOURCE_DIR}/sqlite3.c)
    set_source_files_properties(${sqlite3_src_SOURCE_DIR}/sqlite3.c PROPERTIES
        SKIP_PRECOMPILE_HEADERS ON
        COMPILE_OPTIONS         "-Wno-double-promotion;-Wno-null-dereference"
    )
    target_include_directories(cflib_db PRIVATE ${sqlite3_src_SOURCE_DIR})
    target_link_libraries(cflib_db PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

    cf_find_sources(SQLITE_SOURCES sqlite)
    target_sources(cflib_db PRIVATE ${SQLITE_SOURCES})
    target_compile_definitions(cflib_db PRIVATE ENABLE_SQLITE)
endif()
