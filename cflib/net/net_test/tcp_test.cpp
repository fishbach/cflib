/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/crypt_test/certs.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/test.h>

using namespace cflib::crypt;
using namespace cflib::net;

namespace {

QSemaphore msgSem;
QStringList msgs;
QMutex mutex;

void msg(const QString & msg)
{
    QMutexLocker ml(&mutex);
    msgs << msg;
    msgSem.release();
}

class ServerConn : public TCPConn
{
public:
    ServerConn(TCPConnData * data) :
        TCPConn(data)
    {
        msg(QString("srv new: %1").arg(QString::fromLatin1(peerIP())));
        startReadWatcher();
    }

    ~ServerConn()
    {
        msg("srv deleted");
    }

protected:
    virtual void newBytesAvailable()
    {
        QByteArray in = read();
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
        msg(QString("srv closed: %1").arg((int)type));
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

    QList<TCPConn *> conns;

protected:
    virtual void newConnection(TCPConnData * data)
    {
        conns << new ServerConn(data);
    }
};

class ClientConn : public TCPConn
{
public:
    ClientConn(TCPConnData * data) :
        TCPConn(data)
    {
        msg(QString("cli new: %1:%2").arg(QString::fromLatin1(peerIP())).arg(peerPort()));
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
        msg(QString("cli closed: %1").arg((int)type));
    }

    virtual void writeFinished()
    {
        msg("cli writeFinished");
    }
};

}

class TCP_Test: public QObject
{
    Q_OBJECT
private slots:

    void test_writerClose()
    {
        Server serv;
        QVERIFY(serv.start("127.0.0.1", 12301));
        TCPManager cli;
        TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli new: 127.0.0.1:12301"));
        QVERIFY(msgs.contains("srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("1st msg", true);
        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli writeFinished"));
        QVERIFY(msgs.contains("srv read: 1st msg"));
        msgs.clear();

        conn->write("ping 1");
        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("srv read: ping 1"));
        QVERIFY(msgs.contains("cli read: pong 1"));
        msgs.clear();

        conn->write("ping 2");
        conn->close(TCPConn::WriteClosed);
        conn->write("no msg");
        msgSem.acquire(3);
        QCOMPARE(msgs.size(), 3);
        QVERIFY(msgs.contains("srv read: ping 2"));
        QVERIFY(msgs.contains("cli read: pong 2"));
        QVERIFY(msgs.contains("srv closed: 1"));
        msgs.clear();

        conn->close();
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("cli closed: 3"));
        msgs.clear();

        delete conn;
        foreach (TCPConn * sc, serv.conns) delete sc;
        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli deleted"));
        QVERIFY(msgs.contains("srv deleted"));
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
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli new: 127.0.0.1:12301"));
        QVERIFY(msgs.contains("srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("1st msg");
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("srv read: 1st msg"));
        msgs.clear();

        conn->write("close");
        msgSem.acquire(3);
        QCOMPARE(msgs.size(), 3);
        QVERIFY(msgs.contains("srv read: close"));
        QVERIFY(msgs.contains("srv closed: 3"));
        QVERIFY(msgs.contains("cli closed: 1"));
        msgs.clear();

        delete conn;
        foreach (TCPConn * sc, serv.conns) delete sc;
        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli deleted"));
        QVERIFY(msgs.contains("srv deleted"));
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
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli new: 127.0.0.1:12301"));
        QVERIFY(msgs.contains("srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("1st msg");
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("srv read: 1st msg"));
        msgs.clear();

        conn->write("hard");

        msgSem.acquire(3);
        QCOMPARE(msgs.size(), 3);
        QVERIFY(msgs.contains("srv read: hard"));
        QVERIFY(msgs.contains("srv closed: 7"));
        QVERIFY(msgs.contains("cli closed: 7"));
        msgs.clear();

        delete conn;
        foreach (TCPConn * sc, serv.conns) delete sc;
        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli deleted"));
        QVERIFY(msgs.contains("srv deleted"));
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
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli new: 127.0.0.1:12301"));
        QVERIFY(msgs.contains("srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("writeclose");
        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("srv read: writeclose"));
        QVERIFY(msgs.contains("cli closed: 1"));
        msgs.clear();

        conn->write("1st msg");
        delete conn;
        msgSem.acquire(3);
        QCOMPARE(msgs.size(), 3);
        QVERIFY(msgs.contains("cli deleted"));
        QVERIFY(msgs.contains("srv read: 1st msg"));
        QVERIFY(msgs.contains("srv closed: 3"));
        msgs.clear();

        foreach (TCPConn * sc, serv.conns) delete sc;
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("srv deleted"));
        msgs.clear();
    }

    void test_connectionRefused()
    {
        TCPManager cli;
        TCPConnData * data = cli.openConnection("127.0.0.1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli new: 127.0.0.1:12301"));
        QVERIFY(msgs.contains("cli closed: 7"));
        msgs.clear();

        conn->write("no msg", true);
        delete conn;
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("cli deleted"));
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
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli new: 127.0.0.1:12301"));
        QVERIFY(msgs.contains("srv new: 127.0.0.1"));
        msgs.clear();

        conn->write("ping 1", true);
        msgSem.acquire(3);
        QCOMPARE(msgs.size(), 3);
        QVERIFY(msgs.contains("cli writeFinished"));
        QVERIFY(msgs.contains("srv read: ping 1"));
        QVERIFY(msgs.contains("cli read: pong 1"));
        msgs.clear();

        conn->close(TCPConn::ReadClosed);
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("cli closed: 1"));
        msgs.clear();

        delete conn;
        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli deleted"));
        QVERIFY(msgs.contains("srv closed: 1"));
        msgs.clear();

        foreach (TCPConn * sc, serv.conns) delete sc;
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("srv deleted"));
        msgs.clear();
    }

    void test_IPv6()
    {
        // Do we have an IPv6 loopback device?
        if (QProcess::execute("sh", { "-c", "ifconfig | grep -q 'inet6 ::1'" }) != 0) {;
            QSKIP("no IPv6 loopback device");
        }

        Server serv;
        QVERIFY(serv.start("::1", 12301));
        TCPManager cli;
        TCPConnData * data = cli.openConnection("::1", 12301);
        QVERIFY(data != 0);
        ClientConn * conn = new ClientConn(data);

        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli new: ::1:12301"));
        QVERIFY(msgs.contains("srv new: ::1"));
        msgs.clear();

        conn->write("ping 1", true);
        msgSem.acquire(3);
        QCOMPARE(msgs.size(), 3);
        QVERIFY(msgs.contains("cli writeFinished"));
        QVERIFY(msgs.contains("srv read: ping 1"));
        QVERIFY(msgs.contains("cli read: pong 1"));
        msgs.clear();

        conn->close(TCPConn::ReadClosed);
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("cli closed: 1"));
        msgs.clear();

        delete conn;
        msgSem.acquire(2);
        QCOMPARE(msgs.size(), 2);
        QVERIFY(msgs.contains("cli deleted"));
        QVERIFY(msgs.contains("srv closed: 1"));
        msgs.clear();

        foreach (TCPConn * sc, serv.conns) delete sc;
        msgSem.acquire(1);
        QCOMPARE(msgs.size(), 1);
        QVERIFY(msgs.contains("srv deleted"));
        msgs.clear();
    }

//    forever { msgSem.acquire(); QTextStream(stdout) << msgs.join("|") << Qt::endl; }

};
#include "tcp_test.moc"
ADD_TEST(TCP_Test)
