/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/rmiservice.h>

namespace services {

class ChatService : public cflib::net::RMIServiceBase
{
    SERIALIZE_CLASS
public:
    ChatService();
    ~ChatService();

rmi:
    void sendMessage(const String & message);

cfsignals:
    rsig<void (const String & msg), void()> newMessage;
};

}
