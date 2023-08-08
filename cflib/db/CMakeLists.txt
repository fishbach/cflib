# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_lib(cflib_db
	PUBLIC cflib_util
	PUBLIC Qt5::Sql
	PRIVATE PostgreSQL::PostgreSQL
	DIRS impl
	ENABLE_MOC
)

add_subdirectory(db_test)
add_subdirectory(migrationmoped)
