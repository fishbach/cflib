# Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

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
BOTAN_MAJOR_VERSION = 3
BOTAN_MINOR_VERSION = 1
BOTAN_PATCH_VERSION = 1
!defined(BOTAN_DIR, var) {
	BOTAN_DIR = $${CFLIB_DIR}/../Botan-$${BOTAN_MAJOR_VERSION}.$${BOTAN_MINOR_VERSION}.$${BOTAN_PATCH_VERSION}
}
!exists($$BOTAN_DIR/build/obj/lib/tls_server.*) {
	warning("=======================================================================")
	warning(Botan library not found here: $$BOTAN_DIR)
	message("TODO: ... or adjust variable BOTAN_DIR in config.pri")
	message("-----------------------------------------------------------------------")
	message(cd $$shell_path($${CFLIB_DIR}/..))
	message(curl -L http://botan.randombit.net/releases/Botan-$${BOTAN_MAJOR_VERSION}.$${BOTAN_MINOR_VERSION}.$${BOTAN_PATCH_VERSION}.tar.xz | tar Jx)
	message(cd Botan-$${BOTAN_MAJOR_VERSION}.$${BOTAN_MINOR_VERSION}.$${BOTAN_PATCH_VERSION})
	!win32 {
		message(./configure.py --disable-shared)
		message(make -j 10 libbotan-$${BOTAN_MAJOR_VERSION}.a)
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
