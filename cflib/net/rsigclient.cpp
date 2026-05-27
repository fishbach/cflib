/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rsigclient.h"
#include "rmiclient.h"

#include <cstdio>

using namespace cflib::net;

namespace cflib::net {

#include <cstdio>

RSigClientBase::RSigClientBase(RMIClient & client, const String & service, const String & name)
    : client_(client), service_(service), name_(name)
{
}

RSigClientBase::~RSigClientBase()
{
    if (id_ != 0) {
        client_.unregisterRSig(id_);
    }
}

void RSigClientBase::registerRSig()
{
    if (id_ == 0) {
        id_ = client_.registerRSig(service_, name_);
    }
}

void RSigClientBase::unregisterRSig()
{
    if (id_ != 0) {
        client_.unregisterRSig(id_);
        id_ = 0;
    }
}

} // namespace cflib::net
