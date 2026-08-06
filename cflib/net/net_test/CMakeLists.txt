# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_test(net_test cflib_net
    DIRS
        test
    PROPERTIES
        RESOURCE_LOCK net_test_port
    ENABLE_SER
)
