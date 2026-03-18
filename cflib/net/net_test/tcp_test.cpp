/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/crypt_test/certs.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/base/cfconcurrent.h>
#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/test.h>

#include <cstdlib>

using namespace cflib::crypt;
using namespace cflib::net;

namespace {

CFSemaphore msgSem;
CFStringList msgs;
CFMutex mutex;

void msg(const CFString & m)
{
    CFMutexLocker ml(mutex);
    msgs << m;
    msgSem.release();
}

class ServerConn : public TCPConn
{
public:
    ServerConn(TCPConnData * data) :
        TCPConn(data)
    {
        msg("srv new: " + CFString(peerIP()));
        startReadWatcher();
    }

    ~ServerConn()
    {
        msg("srv deleted");
    }

protected:
    virtual void newBytesAvailable()
    {
        CFByteArray in = read();
        msg("srv read: " + in);
        if (in.startsWith("ping")) {
            in.replace(1, 1, "o");
            write(in);
        } else if (in == "close") {
            close(ReadWriteClosed);
        } else if (in == "hard") {
            close(HardClosed);
        } else if (in == "writeclose") {
            close(WriteClosed);
        }
        startReadWatcher();
    }

    virtual void closed(CloseType type)
    {
        msg("srv closed: " + CFString::number((int)type));
    }

    virtual void writeFinished()
    {
        msg("srv writeFinished");
    }
};

class Server : public TCPManager
{
public:
    Server(uint tlsThreadCount = 0) : TCPManager(tlsThreadCount) {}

    CFList<TCPConn *> conns;

protected:
    virtual void newConnection(TCPConnData * data)
    {
        conns.push_back(new ServerConn(data));
    }
};

class ClientConn : public TCPConn
{
public:
    ClientConn(TCPConnData * data) :
        TCPConn(data)
    {
        msg("cli new: " + CFString(peerIP()) + ":" + CFString::number(peerPort()));
        startReadWatcher();
    }

    ~ClientConn()
    {
        msg("cli deleted");
    }

protected:
    virtual void newBytesAvailable()
    {
        msg("cli read: " + read());
        startReadWatcher();
    }

    virtual void closed(CloseType type)
    {
        msg("cli closed: " + CFString::number((int)type));
    }

    virtual void writeFinished()
    {
        msg("cli writeFinished");
    }
};

}

class TCP_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<TCP_Test *>(this);
        return {
            {"test_writerClose",       [self]() { self->test_writerClose(); }},
            {"test_readerClose",       [self]() { self->test_readerClose(); }},
            {"test_hardClose",         [self]() { self->test_hardClose(); }},
            {"test_sendAndDelete",     [self]() { self->test_sendAndDelete(); }},
            {"test_connectionRefused", [self]() { self->test_connectionRefused(); }},
            {"test_encryption",        [self]() { self->test_encryption(); }},
            {"test_IPv6",              [self]() { self->test_IPv6(); }}
        };
    }

    void test_writerClose()
    {
        Server serv;
        QVERIFY(serv.start("127.0.0.1", 12301));
        TCPManager cli;
        TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli new: 127.0.0.1:12301"));
        QVERIFY(cfContains(msgs, "srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("1st msg", true);
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli writeFinished"));
        QVERIFY(cfContains(msgs, "srv read: 1st msg"));
        msgs.clear();

        conn->write("ping 1");
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "srv read: ping 1"));
        QVERIFY(cfContains(msgs, "cli read: pong 1"));
        msgs.clear();

        conn->write("ping 2");
        conn->close(TCPConn::WriteClosed);
        conn->write("no msg");
        msgSem.acquire(3);
        QCOMPARE((int)msgs.size(), 3);
        QVERIFY(cfContains(msgs, "srv read: ping 2"));
        QVERIFY(cfContains(msgs, "cli read: pong 2"));
        QVERIFY(cfContains(msgs, "srv closed: 1"));
        msgs.clear();

        conn->close();
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "cli closed: 3"));
        msgs.clear();

        delete conn;
        for (auto * sc : serv.conns) delete sc;
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli deleted"));
        QVERIFY(cfContains(msgs, "srv deleted"));
        msgs.clear();
    }

    void test_readerClose()
    {
        Server serv;
        QVERIFY(serv.start("127.0.0.1", 12301));
        TCPManager cli;
        TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli new: 127.0.0.1:12301"));
        QVERIFY(cfContains(msgs, "srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("1st msg");
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "srv read: 1st msg"));
        msgs.clear();

        conn->write("close");
        msgSem.acquire(3);
        QCOMPARE((int)msgs.size(), 3);
        QVERIFY(cfContains(msgs, "srv read: close"));
        QVERIFY(cfContains(msgs, "srv closed: 3"));
        QVERIFY(cfContains(msgs, "cli closed: 1"));
        msgs.clear();

        delete conn;
        for (auto * sc : serv.conns) delete sc;
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli deleted"));
        QVERIFY(cfContains(msgs, "srv deleted"));
        msgs.clear();
    }

    void test_hardClose()
    {
        Server serv;
        QVERIFY(serv.start("127.0.0.1", 12301));
        TCPManager cli;
        TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli new: 127.0.0.1:12301"));
        QVERIFY(cfContains(msgs, "srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("1st msg");
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "srv read: 1st msg"));
        msgs.clear();

        conn->write("hard");

        msgSem.acquire(3);
        QCOMPARE((int)msgs.size(), 3);
        QVERIFY(cfContains(msgs, "srv read: hard"));
        QVERIFY(cfContains(msgs, "srv closed: 7"));
        QVERIFY(cfContains(msgs, "cli closed: 7"));
        msgs.clear();

        delete conn;
        for (auto * sc : serv.conns) delete sc;
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli deleted"));
        QVERIFY(cfContains(msgs, "srv deleted"));
        msgs.clear();
    }

    void test_sendAndDelete()
    {
        Server serv;
        QVERIFY(serv.start("127.0.0.1", 12301));
        TCPManager cli;
        TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli new: 127.0.0.1:12301"));
        QVERIFY(cfContains(msgs, "srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("writeclose");
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "srv read: writeclose"));
        QVERIFY(cfContains(msgs, "cli closed: 1"));
        msgs.clear();

        conn->write("1st msg");
        delete conn;
        msgSem.acquire(3);
        QCOMPARE((int)msgs.size(), 3);
        QVERIFY(cfContains(msgs, "cli deleted"));
        QVERIFY(cfContains(msgs, "srv read: 1st msg"));
        QVERIFY(cfContains(msgs, "srv closed: 3"));
        msgs.clear();

        for (auto * sc : serv.conns) delete sc;
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "srv deleted"));
        msgs.clear();
    }

    void test_connectionRefused()
    {
        TCPManager cli;
        TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli new: 127.0.0.1:12301"));
        QVERIFY(cfContains(msgs, "cli closed: 7"));
        msgs.clear();

        conn->write("no msg", true);
        delete conn;
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "cli deleted"));
        msgs.clear();
    }

    void test_encryption()
    {
        TLSCredentials serverCreds;
        QCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        QVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

        Server serv(1);
        QVERIFY(serv.start("127.0.0.1", 12301, serverCreds));

        TCPManager cli(1);
        QCOMPARE((int)cli.clientCredentials().addCerts(cert3, true), 1);
        QCOMPARE((int)cli.clientCredentials().addRevocationLists(cert2Crl), 1);

        TCPConnData * data = cli.openTLSConnection("127.0.0.1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli new: 127.0.0.1:12301"));
        QVERIFY(cfContains(msgs, "srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("ping 1", true);
        msgSem.acquire(3);
        QCOMPARE((int)msgs.size(), 3);
        QVERIFY(cfContains(msgs, "cli writeFinished"));
        QVERIFY(cfContains(msgs, "srv read: ping 1"));
        QVERIFY(cfContains(msgs, "cli read: pong 1"));
        msgs.clear();

        conn->close(TCPConn::ReadClosed);
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "cli closed: 1"));
        msgs.clear();

        delete conn;
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli deleted"));
        QVERIFY(cfContains(msgs, "srv closed: 1"));
        msgs.clear();

        for (auto * sc : serv.conns) delete sc;
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "srv deleted"));
        msgs.clear();
    }

    void test_IPv6()
    {
        if (system("ifconfig | grep -q 'inet6 ::1'") != 0) {
            QSKIP("no IPv6 loopback device");
        }

        Server serv;
        QVERIFY(serv.start("::1", 12301));
        TCPManager cli;
        TCPConnData * data = cli.openConnection("::1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli new: ::1:12301"));
        QVERIFY(cfContains(msgs, "srv new: ::1"));
        msgs.clear();

        conn->write("ping 1", true);
        msgSem.acquire(3);
        QCOMPARE((int)msgs.size(), 3);
        QVERIFY(cfContains(msgs, "cli writeFinished"));
        QVERIFY(cfContains(msgs, "srv read: ping 1"));
        QVERIFY(cfContains(msgs, "cli read: pong 1"));
        msgs.clear();

        conn->close(TCPConn::ReadClosed);
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "cli closed: 1"));
        msgs.clear();

        delete conn;
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "cli deleted"));
        QVERIFY(cfContains(msgs, "srv closed: 1"));
        msgs.clear();

        for (auto * sc : serv.conns) delete sc;
        msgSem.acquire(1);
        QCOMPARE((int)msgs.size(), 1);
        QVERIFY(cfContains(msgs, "srv deleted"));
        msgs.clear();
    }
};

ADD_TEST(TCP_Test)
