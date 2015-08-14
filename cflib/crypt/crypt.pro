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
!defined(BOTAN_LIB, var) {
	BOTAN_VERSION = 1.11.19
	BOTAN_LIB     = $${CFLIB_DIR}/../Botan-$${BOTAN_VERSION}/libbotan-1.11.a
	BOTAN_INCLUDE = $${CFLIB_DIR}/../Botan-$${BOTAN_VERSION}/build/include
}
!exists($$BOTAN_LIB) {
	warning(Botan library not found here: $$BOTAN_LIB)
	message("TODO: ... or adjust variables BOTAN_LIB and BOTAN_INCLUDE in config.pri")
	message(cd $${CFLIB_DIR}/..)
	message(wget http://botan.randombit.net/releases/Botan-$${BOTAN_VERSION}.tgz)
	message(tar zxvf Botan-$${BOTAN_VERSION}.tgz)
	message(cd Botan-$${BOTAN_VERSION})
	message(./configure.py)
	message(make)
}

# add botan object files
exists(botan):system(chmod u+w botan ; rm -rf botan)
system(mkdir botan ; cd botan ; ar x $$BOTAN_LIB ; chmod u-w .)
OBJECTS += $$getFiles(botan)

INCLUDEPATH += $$BOTAN_INCLUDE
CONFIG      += exceptions
