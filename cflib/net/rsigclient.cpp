/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rsigclient.h"

using namespace cflib::serialize;
using namespace cflib::net;

namespace cflib::net {

RSigClientBase::RSigClientBase(RMIRemoteService & service, const String & name) :
    service_(service), name_(name)
{
}

ByteArray RSigClientBase::unregData() const
{
    BERSerializer ser = service_.getSer();
    ser << name_ << false << id_;
    return ser.data();;
}

} // namespace cflib::net
