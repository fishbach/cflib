# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

find_package(Qt5 COMPONENTS Network)

cf_app(getssl cflib_net Qt5::Network ENABLE_MOC)
