# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../include.pri)

QT = core
CONFIG += console

TARGET = request

HEADERS = \

SOURCES = \
	main.cpp \

useLibs(cflib_net cflib_crypt cflib_util)
app()
