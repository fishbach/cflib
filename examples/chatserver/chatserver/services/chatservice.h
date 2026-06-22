/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <chatserver/dao/message.h>

#include <cflib/dao/version.h>
#include <cflib/net/rmiservice.h>

namespace chatserver::services {

class ChatService : public cflib::net::RMIServiceBase
{
    SERIALIZE_CLASS
public:
    ChatService();
    ~ChatService();

rmi:
    void sendMessage(const dao::Message & message);
    bool test() { return true; }
    bool test2(int & i) { i = 7; return true; }
    void test3(int & i) { i = 13; }
    cflib::dao::Version version() { return cflib::dao::Version{1, 2, 3, "a1"}; }

cfsignals:
    rsig<void (const dao::Message & msg), void()> newMessage;
};

}
