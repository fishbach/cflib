# Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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

include(../../include.pri)

QT = core

TARGET = cflib_crypt

HEADERS = \
	impl/botanhelper.h \
	tlsclient.h \
	tlscredentials.h \
	tlsserver.h \
	tlssessions.h \
	tlsstream.h \
	util.h \

OTHER_FILES = \
	patch_botan.sh \

lib()

# botan
CONFIG += exceptions
macx {
	BOTAN_OS = macosx
	SOURCES += botan/$$BOTAN_OS/botan_all_rdrand.cpp
}
INCLUDEPATH += botan/$$BOTAN_OS
HEADERS += botan/$$BOTAN_OS/botan_all.h botan/$$BOTAN_OS/botan_all_internal.h
SOURCES += botan/$$BOTAN_OS/botan_all.cpp
