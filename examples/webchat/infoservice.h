/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <dao.h>

#include <cflib/net/rmiservice.h>

class InfoService : public cflib::net::RMIService<String>
{
    SERIALIZE_CLASS
public:
    InfoService();
    ~InfoService();

rmi:
    String test();
    void test(int &, int &) {}
    String test(const String & msg);
    void async(cfint64 i);
    cfint64 iTest(cfint64 i) { return i; }
    Dao update(const Dao & dao);
    void update(Dao2 & dao);
    void update(Dao3 & dao);
    void doSignal(int t) { mySig(t); }

    void talk(const String & msg);

cfsignals:
    rsig<void (int t), void()> mySig;
    rsig<void (int t, const String & s), void()> mySig2;
    rsig<void (int t, const String & s), void()> newMessage;
};
