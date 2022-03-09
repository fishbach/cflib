# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../../include.pri)

QT = core
CONFIG += console no_pch

TARGET = jscombiner

SOURCES = \
	main.cpp \

useLibs(cflib_util)
app()
