# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../include.pri)

QT = core gui websockets widgets

TARGET = webchatclient

HEADERS += \
	chatwindow.h \

SOURCES = \
	chatwindow.cpp \
	main.cpp \

FORMS += \
	chatwindow.ui \


serializeGen()
useLibs(cflib_net cflib_crypt cflib_serialize cflib_util)
app()
