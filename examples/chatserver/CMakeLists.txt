# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_app(chatserver
        chatserver_dao
        cflib_net
    DIRS
        chatserver/services
    ENABLE_SER
)

add_subdirectory(chatserver/dao)
add_subdirectory(remote)
