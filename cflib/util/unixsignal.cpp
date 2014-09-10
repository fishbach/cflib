/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#include "unixsignal.h"

#include <cflib/util/log.h>

#ifdef Q_OS_UNIX
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

USE_LOG(LogCat::Etc)

namespace cflib { namespace util {

namespace {

bool active = false;

#ifdef Q_OS_UNIX

int sockets[2];
sig_t oldSigH1 = 0;
sig_t oldSigH2 = 0;
sig_t oldSigH15 = 0;

void signalHandler(int sig)
{
	char s = (char)sig;
	::write(sockets[0], &s, 1);
}

#endif

}

UnixSignal::UnixSignal()
{
	logFunctionTrace

	if (active) qFatal("only one UnixSignal instance can exist");
	active = true;

#ifdef Q_OS_UNIX
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets)) qFatal("Couldn't create socketpair");
	sn_ = new QSocketNotifier(sockets[1], QSocketNotifier::Read);
	connect(sn_, SIGNAL(activated(int)), this, SLOT(activated()));
	oldSigH1  = ::signal(1,  signalHandler);
	oldSigH2  = ::signal(2,  signalHandler);
	oldSigH15 = ::signal(15, signalHandler);
#endif
}

UnixSignal::~UnixSignal()
{
	logFunctionTrace

#ifdef Q_OS_UNIX
	::signal(1,  oldSigH1);
	::signal(2,  oldSigH2);
	::signal(15, oldSigH15);
	delete sn_;
	::close(sockets[0]);
	::close(sockets[1]);
#endif

	active = false;
}

void UnixSignal::activated()
{
#ifdef Q_OS_UNIX
	char s;
	::read(sockets[1], &s, 1);
	int sig = s;
	logInfo("catched signal %1", sig);
	emit catchedSignal(sig);
#endif
}

}}	// namespace
