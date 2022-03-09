/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rmiservice.h"

#include <cflib/net/impl/rmiserverbase.h>

namespace cflib { namespace net {

void RSigBase::send(uint connId, const QByteArray & data)
{
	server_->send(connId, data);
}

}}	// namespace
