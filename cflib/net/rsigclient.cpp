/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rsigclient.h"

namespace cflib::net {

RSigClientBase::RSigClientBase(RMIRemoteService & service, const String & name) :
    service_(service), name_(name)
{
}

RSigClientBase::~RSigClientBase()
{
    unreg();
}

void RSigClientBase::unreg()
{
    if (id_ == 0) return;
    serialize::BERSerializer ser = service_.getSer();
    ser << name_ << false << id_;
    service_.unregisterRSig(id_, ser.data());
    id_ = 0;
    regData_.clear();
}

} // namespace cflib::net
