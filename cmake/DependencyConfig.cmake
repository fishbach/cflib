# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

# Botan
find_package(
	Botan 3.1.1
	REQUIRED
)

# PostgreSQL
if(ENABLE_PSQL)
	find_package(PostgreSQL REQUIRED)
endif()

# ZLIB
find_package(ZLIB REQUIRED)
