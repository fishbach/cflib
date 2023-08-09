# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

add_subdirectory(getssl)
if(ENABLE_PSQL)
	add_subdirectory(pgtest)
endif()
add_subdirectory(request)
add_subdirectory(webchat)
add_subdirectory(webchatclient)
