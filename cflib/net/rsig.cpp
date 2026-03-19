/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rmiservice.h"

#include <cflib/net/impl/rmiserverbase.h>

namespace cflib::net {

void RSigBase::send(uint connId, const ByteArray & data)
{
    server_->send(connId, data);
}

} // namespace
