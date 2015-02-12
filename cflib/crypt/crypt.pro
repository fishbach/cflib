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

# botan
include(botan/botan.pri)
win32 {
	HEADERS += \
		botan/internal/es_capi.h \
		botan/internal/es_win32.h \
}
unix {
	HEADERS += \
		botan/internal/dev_random.h \
		botan/internal/es_egd.h \
		botan/internal/proc_walk.h \
		botan/internal/unix_procs.h \
		botan/locking_allocator.h \

	SOURCES += \
		botan/internal/unix_proc_sources.cpp \
}

lib()

# botan
CONFIG += exceptions
unix {
	DEFINES += \
		BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM \
		BOTAN_HAS_ENTROPY_SRC_EGD \
		BOTAN_HAS_ENTROPY_SRC_PROC_WALKER \
		BOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER \
		BOTAN_HAS_LOCKING_ALLOCATOR \
		BOTAN_TARGET_OS_HAS_GMTIME_R \
}
win32 {
	DEFINES += \
		BOTAN_HAS_ENTROPY_SRC_CAPI \
		BOTAN_HAS_ENTROPY_SRC_WIN32 \
		BOTAN_TARGET_OS_HAS_GMTIME_S \
}
