# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../../include.pri)

QT = core

TARGET = util_test

SOURCES = \
	sig_test.cpp \
	threadfifo_test.cpp \
	util_test.cpp \

serializeGen()
useLibs(cflib_util)
test()
