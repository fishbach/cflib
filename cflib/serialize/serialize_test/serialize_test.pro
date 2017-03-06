# Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# cflib is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# cflib is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cflib. If not, see <http://www.gnu.org/licenses/>.

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
