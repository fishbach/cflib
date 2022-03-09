# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../include.pri)

QT = core network
CONFIG += console

TARGET = getssl

HEADERS = \
	letsencrypt.h \

SOURCES = \
	letsencrypt.cpp \
	main.cpp \

useLibs(cflib_net cflib_crypt cflib_util)
app()
