# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../../include.pri)

QT = core

TARGET = net_test

SOURCES = \
	http_test.cpp \
	tcp_test.cpp \

useLibs(cflib_net cflib_crypt cflib_util)
test()
