/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/crypt_test/certs.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/base.h>
#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/test.h>

#include <cstdlib>

using namespace cflib::crypt;
using namespace cflib::net;

namespace {

Semaphore msgSem;
StringList msgs;
Mutex mutex;

void msg(const String & m)
{
    MutexLocker ml(mutex);
    msgs << m;
    msgSem.release();
}

class ServerConn : public TCPConn
{
public:
    ServerConn(TCPConnData * data) :
        TCPConn(data)
    {
        msg("srv new: " + String(peerIP()));
        startReadWatcher();
    }

    ~ServerConn()
    {
        msg("srv deleted");
    }

protected:
    virtual void newBytesAvailable()
    {
        ByteArray in = read();
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
        msg("srv closed: " + String::number((int)type));
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

    List<TCPConn *> conns;

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
        msg("cli new: " + String(peerIP()) + ":" + String::number(peerPort()));
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
        msg("cli closed: " + String::number((int)type));
    }

    virtual void writeFinished()
    {
        msg("cli writeFinished");
    }
};

}

TEST_SUITE("TCP") {

TEST_CASE("TCP: writerClose")
{
    Server serv;
    REQUIRE(serv.start("127.0.0.1", 12301));
    TCPManager cli;
    TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
    REQUIRE(data != 0);
    ClientConn * conn = new ClientConn(data);

    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli new: 127.0.0.1:12301"));
    REQUIRE(msgs.contains("srv new: 127.0.0.1"));
    msgs.clear();

    conn->write("1st msg", true);
    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli writeFinished"));
    REQUIRE(msgs.contains("srv read: 1st msg"));
    msgs.clear();

    conn->write("ping 1");
    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("srv read: ping 1"));
    REQUIRE(msgs.contains("cli read: pong 1"));
    msgs.clear();

    conn->write("ping 2");
    conn->close(TCPConn::WriteClosed);
    conn->write("no msg");
    msgSem.acquire(3);
    REQUIRE_EQ((int)msgs.size(), 3);
    REQUIRE(msgs.contains("srv read: ping 2"));
    REQUIRE(msgs.contains("cli read: pong 2"));
    REQUIRE(msgs.contains("srv closed: 1"));
    msgs.clear();

    conn->close();
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("cli closed: 3"));
    msgs.clear();

    delete conn;
    for (auto * sc : serv.conns) delete sc;
    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli deleted"));
    REQUIRE(msgs.contains("srv deleted"));
    msgs.clear();
}

TEST_CASE("TCP: readerClose")
{
    Server serv;
    REQUIRE(serv.start("127.0.0.1", 12301));
    TCPManager cli;
    TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
    REQUIRE(data != 0);
    ClientConn * conn = new ClientConn(data);

    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli new: 127.0.0.1:12301"));
    REQUIRE(msgs.contains("srv new: 127.0.0.1"));
    msgs.clear();

    conn->write("1st msg");
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("srv read: 1st msg"));
    msgs.clear();

    conn->write("close");
    msgSem.acquire(3);
    REQUIRE_EQ((int)msgs.size(), 3);
    REQUIRE(msgs.contains("srv read: close"));
    REQUIRE(msgs.contains("srv closed: 3"));
    REQUIRE(msgs.contains("cli closed: 1"));
    msgs.clear();

    delete conn;
    for (auto * sc : serv.conns) delete sc;
    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli deleted"));
    REQUIRE(msgs.contains("srv deleted"));
    msgs.clear();
}

TEST_CASE("TCP: hardClose")
{
    Server serv;
    REQUIRE(serv.start("127.0.0.1", 12301));
    TCPManager cli;
    TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
    REQUIRE(data != 0);
    ClientConn * conn = new ClientConn(data);

    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli new: 127.0.0.1:12301"));
    REQUIRE(msgs.contains("srv new: 127.0.0.1"));
    msgs.clear();

    conn->write("1st msg");
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("srv read: 1st msg"));
    msgs.clear();

    conn->write("hard");

    msgSem.acquire(3);
    REQUIRE_EQ((int)msgs.size(), 3);
    REQUIRE(msgs.contains("srv read: hard"));
    REQUIRE(msgs.contains("srv closed: 7"));
    REQUIRE(msgs.contains("cli closed: 7"));
    msgs.clear();

    delete conn;
    for (auto * sc : serv.conns) delete sc;
    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli deleted"));
    REQUIRE(msgs.contains("srv deleted"));
    msgs.clear();
}

TEST_CASE("TCP: sendAndDelete")
{
    Server serv;
    REQUIRE(serv.start("127.0.0.1", 12301));
    TCPManager cli;
    TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
    REQUIRE(data != 0);
    ClientConn * conn = new ClientConn(data);

    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli new: 127.0.0.1:12301"));
    REQUIRE(msgs.contains("srv new: 127.0.0.1"));
    msgs.clear();

    conn->write("writeclose");
    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("srv read: writeclose"));
    REQUIRE(msgs.contains("cli closed: 1"));
    msgs.clear();

    conn->write("1st msg");
    delete conn;
    msgSem.acquire(3);
    REQUIRE_EQ((int)msgs.size(), 3);
    REQUIRE(msgs.contains("cli deleted"));
    REQUIRE(msgs.contains("srv read: 1st msg"));
    REQUIRE(msgs.contains("srv closed: 3"));
    msgs.clear();

    for (auto * sc : serv.conns) delete sc;
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("srv deleted"));
    msgs.clear();
}

TEST_CASE("TCP: connectionRefused")
{
    TCPManager cli;
    TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
    REQUIRE(data != 0);
    ClientConn * conn = new ClientConn(data);

    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli new: 127.0.0.1:12301"));
    REQUIRE(msgs.contains("cli closed: 7"));
    msgs.clear();

    conn->write("no msg", true);
    delete conn;
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("cli deleted"));
    msgs.clear();
}

TEST_CASE("TCP: encryption")
{
    TLSCredentials serverCreds;
    REQUIRE_EQ((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
    REQUIRE(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

    Server serv(1);
    REQUIRE(serv.start("127.0.0.1", 12301, serverCreds));

    TCPManager cli(1);
    REQUIRE_EQ((int)cli.clientCredentials().addCerts(cert3, true), 1);
    REQUIRE_EQ((int)cli.clientCredentials().addRevocationLists(cert2Crl), 1);

    TCPConnData * data = cli.openTLSConnection("127.0.0.1", 12301);
    REQUIRE(data != 0);
    ClientConn * conn = new ClientConn(data);

    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli new: 127.0.0.1:12301"));
    REQUIRE(msgs.contains("srv new: 127.0.0.1"));
    msgs.clear();

    conn->write("ping 1", true);
    msgSem.acquire(3);
    REQUIRE_EQ((int)msgs.size(), 3);
    REQUIRE(msgs.contains("cli writeFinished"));
    REQUIRE(msgs.contains("srv read: ping 1"));
    REQUIRE(msgs.contains("cli read: pong 1"));
    msgs.clear();

    conn->close(TCPConn::ReadClosed);
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("cli closed: 1"));
    msgs.clear();

    delete conn;
    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli deleted"));
    REQUIRE(msgs.contains("srv closed: 1"));
    msgs.clear();

    for (auto * sc : serv.conns) delete sc;
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("srv deleted"));
    msgs.clear();
}

TEST_CASE("TCP: IPv6")
{
    if (system("ifconfig | grep -q 'inet6 ::1'") != 0) {
        MESSAGE("SKIP: no IPv6 loopback device");
        return;
    }

    Server serv;
    REQUIRE(serv.start("::1", 12301));
    TCPManager cli;
    TCPConnData * data = cli.openConnection("::1", 12301);
    REQUIRE(data != 0);
    ClientConn * conn = new ClientConn(data);

    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli new: ::1:12301"));
    REQUIRE(msgs.contains("srv new: ::1"));
    msgs.clear();

    conn->write("ping 1", true);
    msgSem.acquire(3);
    REQUIRE_EQ((int)msgs.size(), 3);
    REQUIRE(msgs.contains("cli writeFinished"));
    REQUIRE(msgs.contains("srv read: ping 1"));
    REQUIRE(msgs.contains("cli read: pong 1"));
    msgs.clear();

    conn->close(TCPConn::ReadClosed);
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("cli closed: 1"));
    msgs.clear();

    delete conn;
    msgSem.acquire(2);
    REQUIRE_EQ((int)msgs.size(), 2);
    REQUIRE(msgs.contains("cli deleted"));
    REQUIRE(msgs.contains("srv closed: 1"));
    msgs.clear();

    for (auto * sc : serv.conns) delete sc;
    msgSem.acquire(1);
    REQUIRE_EQ((int)msgs.size(), 1);
    REQUIRE(msgs.contains("srv deleted"));
    msgs.clear();
}

}
