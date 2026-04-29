/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/wscommmanager.h>
#include <cflib/util/evtimer.h>

namespace cflib::net {

class AliveSender : public util::ThreadVerify
{
public:
    AliveSender(WSCommManagerBase & mgr, double interval = 1.0);
    ~AliveSender();

    void initThreadData() override;

private:
    void sendAlive();

private:
    WSCommManagerBase & mgr_;
    const double interval_;
    util::EVTimer aliveTimer_;
};

} // namespace
