# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_app(chatserver
        cflib_net
    DIRS
        services
    ENABLE_SER
)

add_custom_command(TARGET chatserver POST_BUILD
    COMMAND chatserver --export "${CMAKE_CURRENT_SOURCE_DIR}/api"
)
