/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "alivesender.h"

USE_LOG(LogCat::Http)

namespace cflib::net {

AliveSender::AliveSender(WSCommManagerBase & mgr, double interval) :
    ThreadVerify("AliveService", Worker),
    mgr_(mgr),
    interval_(interval),
    aliveTimer_(this, &AliveSender::sendAlive)
{
}

AliveSender::~AliveSender()
{
    stopVerifyThread();
}

void AliveSender::initThreadData()
{
    aliveTimer_.start(interval_);
}

void AliveSender::sendAlive()
{
    static const ByteArray data = serialize::BERSerializer(5).data();
    for (uint connId : mgr_.getAllConnIds()) mgr_.send(connId, data, true);
}

} // namespace
