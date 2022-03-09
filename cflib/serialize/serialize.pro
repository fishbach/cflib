# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.
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

include(../../include.pri)

QT = core

TARGET = cflib_serialize

HEADERS = \
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
