# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../../include.pri)

QT = core

TARGET = serialize_test

HEADERS = \
	dynamic.h \
	gentest.h \
	test.h \
	test/gentest2.h \

SOURCES = \
	dynamic_test.cpp \
	gen_ber_test.cpp \
	serializeber_test.cpp \
	typeinfo_test.cpp \
	util_test.cpp \

serializeGen()
useLibs(cflib_serialize cflib_util)
test()
