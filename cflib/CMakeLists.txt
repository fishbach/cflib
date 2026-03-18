# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

add_subdirectory(base)
add_subdirectory(crypt)
add_subdirectory(dao)
if(ENABLE_PSQL)
    add_subdirectory(db)
endif()
add_subdirectory(net)
add_subdirectory(serialize)
add_subdirectory(util)
