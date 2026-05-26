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
    fprintf(stderr, "RSigClientBase constructor: client=%p, service=%s\n", &client, service.toUtf8().constData());
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

template<typename... P, typename... R>
RSigClient<void (P...), void (R...)>::RSigClient(RMIClient & client, const String & service, const String & name)
    : RSigClientBase(client, service, name)
{
    // registerRSig();
    // Register later manually
}

template<typename... P, typename... R>
RSigClient<void (P...), void (R...)>::~RSigClient()
{
    unregisterRSig();
}

template class RSigClient<void (const String &), void ()>;

} // namespace cflib::net
