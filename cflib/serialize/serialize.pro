# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../include.pri)

QT = core

TARGET = cflib_serialize

HEADERS = \
	asn1dump.h \
	common.h \
	impl/ber.h \
	impl/readandcall.h \
	impl/registerclass.h \
	impl/serializebaseber.h \
	impl/serializeberimpl.h \
	impl/serializetypeinfoimpl.h \
	serialize.h \
	serializeber.h \
	serializetypeinfo.h \
	tribool.h \
	util.h \

SOURCES = \
	serializetypeinfo.cpp \

lib()
