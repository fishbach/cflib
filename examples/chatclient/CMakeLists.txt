# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_app(chatclient
        ChatServer::services
        cflib_net
    ENABLE_SER
)
