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
	impl/tls12policy.h \
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
BOTAN_MINOR_VERSION = 25
!defined(BOTAN_DIR, var) {
	BOTAN_DIR = $${CFLIB_DIR}/../Botan-1.11.$${BOTAN_MINOR_VERSION}
}
!exists($$BOTAN_DIR/build/obj/lib/tls_server.*) {
	warning("=======================================================================")
	warning(Botan library not found here: $$BOTAN_DIR)
	message("TODO: ... or adjust variable BOTAN_DIR in config.pri")
	message("-----------------------------------------------------------------------")
	message(cd $$shell_path($${CFLIB_DIR}/..))
	message(wget http://botan.randombit.net/releases/Botan-1.11.$${BOTAN_MINOR_VERSION}.tgz)
	message(tar zxvf Botan-1.11.$${BOTAN_MINOR_VERSION}.tgz)
	message(cd Botan-1.11.$${BOTAN_MINOR_VERSION})
	!win32 {
		message(./configure.py --disable-shared)
		message(make -j 10 libbotan-1.11.a)
		message(chmod a-w build/obj/lib)
	} else {
		message(python.exe configure.py --disable-shared)
		message(nmake botan.lib)
		message(attrib +r built\obj\lib\*)
	}
	warning("=======================================================================")
}

# add botan object files
OBJECTS += $$getFiles($$BOTAN_DIR/build/obj/lib)

INCLUDEPATH += $${BOTAN_DIR}/build/include
CONFIG      += exceptions
