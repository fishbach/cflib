# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../include.pri)

QT = core

TARGET = cflib_dao

HEADERS = \
	config.h \
	version.h \

postgresql {
	HEADERS += \
		configpsql.h \

}

serializeGen()
lib()
