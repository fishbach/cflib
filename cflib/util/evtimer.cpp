/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "evtimer.h"

#include <cflib/util/libev.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>

USE_LOG(LogCat::Etc)

namespace cflib { namespace util {

void EVTimer::init()
{
    timer_ = new ev_timer;
    ev_init(timer_, &EVTimer::timeout);
    timer_->data = this;
}

EVTimer::~EVTimer()
{
    delete timer_;
    delete func_;
}

void EVTimer::start(double after, double repeat)
{
    ev_timer_set(timer_, after, repeat);
    ev_timer_start(libEVLoopOfThread(), timer_);
}

void EVTimer::stop()
{
    ev_timer_stop(libEVLoopOfThread(), timer_);
}

void EVTimer::timeout(ev_loop *, ev_timer * w, int)
{
    EVTimer * timer = (EVTimer *)w->data;
    (*timer->func_)();
}

}}    // namespace
