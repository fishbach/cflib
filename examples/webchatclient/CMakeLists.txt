# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

cf_find_qt(WebSockets Widgets)

cf_app(webchatclient cflib_net Qt5::WebSockets Qt5::Widgets ENABLE_MOC ENABLE_UIC)
