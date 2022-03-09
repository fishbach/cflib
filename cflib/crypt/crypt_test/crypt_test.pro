# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../../include.pri)

QT = core

TARGET = crypt_test

HEADERS = \
	certs.h \

SOURCES = \
	tlscredentials_test.cpp \
	tlsserver_test.cpp \
	util_test.cpp \

serializeGen()
useLibs(cflib_crypt cflib_util)
test()
