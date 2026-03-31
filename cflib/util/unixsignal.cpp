/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "unixsignal.h"

#include <cflib/util/libev.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>

#include <csignal>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>

USE_LOG(LogCat::Etc)

namespace cflib::util {

namespace {

bool active = false;
int sockets[2];
sig_t oldSigH1 = 0;
sig_t oldSigH2 = 0;
sig_t oldSigH15 = 0;
UnixSignal * currentInstance = nullptr;

void signalHandler(int sig)
{
    char s = (char)sig;
    int c __attribute__((unused)) = ::write(sockets[0], &s, 1);
}

}

UnixSignal::UnixSignal()
    : watcher_(nullptr)
{
    logFunctionTrace

    if (active) {
        logCritical("only one UnixSignal instance can exist");
        abort();
    }
    active = true;
    currentInstance = this;

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets)) {
        logCritical("Couldn't create socketpair");
        abort();
    }

    ev_loop * loop = libEVLoop();
    if (loop) {
        watcher_ = new ev_io;
        ev_io_init(watcher_, &UnixSignal::ioCallback, sockets[1], EV_READ);
        watcher_->data = this;
        ev_io_start(loop, watcher_);
    }

    oldSigH1  = ::signal(1,  signalHandler);
    oldSigH2  = ::signal(2,  signalHandler);
    oldSigH15 = ::signal(15, signalHandler);
}

UnixSignal::~UnixSignal()
{
    logFunctionTrace

    ::signal(1,  oldSigH1);
    ::signal(2,  oldSigH2);
    ::signal(15, oldSigH15);

    if (watcher_) {
        ev_loop * loop = libEVLoop();
        if (loop) ev_io_stop(loop, watcher_);
        delete watcher_;
    }

    ::close(sockets[0]);
    ::close(sockets[1]);

    currentInstance = nullptr;
    active = false;
}

void UnixSignal::ioCallback(ev_loop *, ev_io * w, int)
{
    UnixSignal * self = (UnixSignal *)w->data;
    char s;
    int c __attribute__((unused)) = ::read(sockets[1], &s, 1);
    int sig = s;
    logInfo("catched signal %1", sig);
    self->catchedSignal(sig);
}

} // namespace
