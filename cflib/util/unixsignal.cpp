/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "unixsignal.h"

#include <cflib/util/libev.h>
#include <cflib/util/log.h>
#include <cflib/util/mainloop.h>
#include <cflib/util/threadverify.h>

#ifdef CF_OS_UNIX
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

USE_LOG(LogCat::Etc)

namespace cflib::util {

namespace {

bool active = false;

#ifdef CF_OS_UNIX

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

UnixSignal::UnixSignal(bool exitMainLoop) :
    exitMainLoop_(exitMainLoop),
    watcher_(nullptr)
{
    if (active) {
        logCritical("only one UnixSignal instance can exist");
        abort();
    }
    active = true;

    ev_loop * loop = libEVLoop();
    if (!loop) {
        logCritical("there must be an existing MainLoop instance");
        abort();
    }

    #ifdef CF_OS_UNIX
        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets)) {
            logCritical("Couldn't create socketpair");
            abort();
        }

        watcher_ = new ev_io;
        ev_io_init(watcher_, &UnixSignal::ioCallback, sockets[1], EV_READ);
        watcher_->data = this;
        ev_io_start(loop, watcher_);

        oldSigH1  = ::signal(1,  signalHandler);
        oldSigH2  = ::signal(2,  signalHandler);
        oldSigH15 = ::signal(15, signalHandler);

        logInfo("installed handler for signals 1, 2, 15");
    #else
        CF_UNUSED(quitMainLoop)
    #endif
}

UnixSignal::~UnixSignal()
{
    #ifdef CF_OS_UNIX
        ::signal(1,  oldSigH1);
        ::signal(2,  oldSigH2);
        ::signal(15, oldSigH15);

        ev_io_stop(libEVLoop(), watcher_);
        delete watcher_;

        ::close(sockets[0]);
        ::close(sockets[1]);
    #endif

    active = false;
}

void UnixSignal::gotSignal(int sig)
{
    logInfo("catched signal %1", sig);
    catchedSignal(sig);
    if (exitMainLoop_) MainLoop::exit();
}

void UnixSignal::ioCallback(ev_loop *, ev_io * w, int)
{
    UnixSignal * self = (UnixSignal *)w->data;
    char s;
    ::read(sockets[1], &s, 1);
    self->gotSignal(s);
}

} // namespace
