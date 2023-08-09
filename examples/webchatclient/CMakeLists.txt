# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

find_package(
	Qt5
	COMPONENTS
		WebSockets
		Widgets
	REQUIRED
)

cf_app(webchatclient cflib_net Qt5::WebSockets Qt5::Widgets ENABLE_MOC ENABLE_UIC)
