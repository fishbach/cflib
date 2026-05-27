# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_app(chatserver
        cflib_net
    DIRS
        chatserver/services
    DAO
        chatserver/dao
    REMOTE
        remote
    ENABLE_SER
)
