/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <dao.h>

#include <cflib/net/rmiservice.h>

class InfoService : public cflib::net::RMIService<QString>
{
    SERIALIZE_CLASS
public:
    InfoService();
    ~InfoService();

rmi:
    QString test();
    void test(int &, int &) {}
    QString test(const QString & msg);
    void async(qint64 i);
    qint64 iTest(qint64 i) { return i; }
    Dao update(const Dao & dao);
    void update(Dao2 & dao);
    void update(Dao3 & dao);
    void doSignal(int t) { mySig(t); }

    void talk(const QString & msg);

cfsignals:
    rsig<void (int t), void()> mySig;
    rsig<void (int t, const QString & s), void()> mySig2;
    rsig<void (int t, const QString & s), void()> newMessage;
};
