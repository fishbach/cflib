# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

set(sqlite_version_year 2026)

# format version for download (3.51.3 -> 3510300)
string(REPLACE "." ";" sqlite_version_parts "${SQLite3_FIND_VERSION}")
list(LENGTH sqlite_version_parts sqlite_version_n)
if(sqlite_version_n LESS 4)
    math(EXPR sqlite_version_n "3 - ${sqlite_version_n}")
    foreach(_i RANGE ${sqlite_version_n})
        list(APPEND sqlite_version_parts 0)
    endforeach()
endif()
list(GET sqlite_version_parts 0 sqlite_file)
foreach(idx 1 2 3)
    list(GET sqlite_version_parts ${idx} part)
    if(part MATCHES "^[0-9]$")
        string(PREPEND part "0")
    endif()
    string(APPEND sqlite_file "${part}")
endforeach()

# get sources
include(FetchContent)
FetchContent_Declare(
    sqlite_src
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL "https://sqlite.org/${sqlite_version_year}/sqlite-amalgamation-${sqlite_file}.zip"
)
FetchContent_MakeAvailable(sqlite_src)

# handle find_package logic
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLite3 DEFAULT_MSG sqlite_src_SOURCE_DIR)

# add target cflib_sqlite
add_library(cflib_sqlite ${sqlite_src_SOURCE_DIR}/sqlite3.c)
target_compile_options(cflib_sqlite PRIVATE
    -Wno-cast-align
    -Wno-double-promotion
    -Wno-null-dereference
)
target_include_directories(cflib_sqlite INTERFACE ${sqlite_src_SOURCE_DIR})
