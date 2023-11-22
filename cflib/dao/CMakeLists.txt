# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

if(ENABLE_PSQL)
    set(PSQL_CONFIG
        DIRS psql
    )
endif()

cf_lib(cflib_dao
    PUBLIC cflib_db
    PUBLIC cflib_serialize
    ${PSQL_CONFIG}
    ENABLE_SER
)
