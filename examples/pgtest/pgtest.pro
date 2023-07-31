# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../include.pri)

QT = core
CONFIG += console

TARGET = pgtest

HEADERS = \

SOURCES = \
	main.cpp \

useLibs(cflib_db cflib_util)
app()
