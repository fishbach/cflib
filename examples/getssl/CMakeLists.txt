# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_find_qt(Network)

cf_app(getssl cflib_net Qt5::Network ENABLE_MOC)
