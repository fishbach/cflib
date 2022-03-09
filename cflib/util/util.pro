# Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

include(../../include.pri)

QT = core

TARGET = cflib_util

HEADERS = \
	cmdline.h \
	delegate.h \
	evtimer.h \
	functor.h \
	hex.h \
	libev.h \
	log.h \
	mailer.h \
	sig.h \
	test.h \
	threadfifo.h \
	threadstats.h \
	threadverify.h \
	timer.h \
	timeshifter.h \
	tuplecompare.h \
	unixsignal.h \
	util.h \
	impl/libevimpl.h \
	impl/logformat.h \
	impl/threadverifyimpl.h \

OTHER_FILES = \
	impl/generate_templates.pl \

lib()

macx: OBJECTIVE_SOURCES += impl/util_mac.mm
