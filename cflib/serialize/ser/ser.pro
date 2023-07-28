# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../../include.pri)

QT = core
CONFIG += console

TARGET = ser

HEADERS = \
	codegen.h \
	headerparser.h \

SOURCES = \
	codegen.cpp \
	headerparser.cpp \
	main.cpp \

useLibs(cflib_util)
app()
