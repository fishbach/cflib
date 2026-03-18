# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_test(db_test cflib_db)
target_compile_definitions(db_test PRIVATE SCHEMA_SQL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/schema.sql")
