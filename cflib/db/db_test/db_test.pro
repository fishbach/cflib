# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../../include.pri)

QT = core

TARGET = db_test

HEADERS = \

SOURCES = \

OTHER_FILES = \
	structure.sql \

postgresql: RESOURCES += \
	db_test.qrc \

postgresql: SOURCES += \
	psql_test.cpp \
	schema_test.cpp \

useLibs(cflib_db cflib_util)
test()
