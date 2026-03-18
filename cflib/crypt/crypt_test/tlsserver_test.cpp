/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/tlsclient.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/tlsserver.h>
#include <cflib/crypt/tlssessions.h>
#include <cflib/crypt/util.h>
#include <cflib/util/test.h>

#include "certs.h"

using namespace cflib::crypt;

class TLSServer_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<TLSServer_test *>(this);
        return {
            {"test_tls",                    [self]() { self->test_tls(); }},
            {"test_fastSend",               [self]() { self->test_fastSend(); }},
            {"test_missingCA",              [self]() { self->test_missingCA(); }},
            {"test_hostname",               [self]() { self->test_hostname(); }},
            {"test_wrongHostname",          [self]() { self->test_wrongHostname(); }},
            {"test_tls_highSecurity_client",[self]() { self->test_tls_highSecurity_client(); }},
            {"test_tls_highSecurity_server",[self]() { self->test_tls_highSecurity_server(); }}
        };
    }

    void test_tls()
    {
        TLSCredentials serverCreds;
        QCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        QVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        QCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        QCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds);

        CFByteArray enc1;
        CFByteArray enc2;
        CFByteArray plain;

        // client starts handshake
        QVERIFY(server.initialSend().isEmpty());
        enc1 = client.initialSend();
        QVERIFY(!enc1.isEmpty());
        QVERIFY(client.initialSend().isEmpty());

        // first handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(enc2.isEmpty());

        // send plain to server
        CFByteArray msg = "hello dear server";
        enc1 = CFByteArray();
        client.send(msg, enc1);
        QVERIFY(!enc1.isEmpty());
        QVERIFY(enc1.indexOf(msg) == -1);
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QCOMPARE(plain, msg);
        QVERIFY(enc2.isEmpty());

        // send plain to client
        msg = "hello dear client";
        enc2 = CFByteArray();
        server.send(msg, enc2);
        QVERIFY(!enc2.isEmpty());
        QVERIFY(enc2.indexOf(msg) == -1);
        plain = CFByteArray();
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QCOMPARE(plain, msg);
        QVERIFY(enc1.isEmpty());
    }

    void test_fastSend()
    {
        TLSCredentials serverCreds;
        QCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        QVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        QCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        QCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds);

        CFByteArray enc1;
        CFByteArray enc2;
        CFByteArray plain;

        // send before handshake
        CFByteArray serverMsg = "hello dear client";
        enc2 = CFByteArray();
        QVERIFY(server.send(serverMsg, enc2));
        QVERIFY(enc2.isEmpty());

        // send before handshake
        CFByteArray clientMsg = "hello dear server";
        enc1 = CFByteArray();
        QVERIFY(client.send(clientMsg, enc1));
        QVERIFY(enc1.isEmpty());

        // client starts handshake
        enc1 = client.initialSend();
        QVERIFY(!enc1.isEmpty());
        QVERIFY(enc1.indexOf(clientMsg) == -1);

        // first handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QCOMPARE(plain, clientMsg);
        plain = CFByteArray();
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QCOMPARE(plain, serverMsg);
        QVERIFY(enc1.isEmpty());
    }

    void test_missingCA()
    {
        TLSCredentials serverCreds;
        QCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        QVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds);

        CFByteArray enc1;
        CFByteArray enc2;
        CFByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        QVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(!client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
    }

    void test_hostname()
    {
        TLSCredentials serverCreds;
        QCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        QVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        QCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        QCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds, "127.0.0.1");

        CFByteArray enc1;
        CFByteArray enc2;
        CFByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        QVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(enc2.isEmpty());

        // send plain to server
        CFByteArray msg = "hello dear server";
        enc1 = CFByteArray();
        QVERIFY(client.send(msg, enc1));
        QVERIFY(!enc1.isEmpty());
        QVERIFY(enc1.indexOf(msg) == -1);
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QCOMPARE(plain, msg);
        QVERIFY(enc2.isEmpty());

        // send plain to client
        msg = "hello dear client";
        enc2 = CFByteArray();
        QVERIFY(server.send(msg, enc2));
        QVERIFY(!enc2.isEmpty());
        QVERIFY(enc2.indexOf(msg) == -1);
        plain = CFByteArray();
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QCOMPARE(plain, msg);
        QVERIFY(enc1.isEmpty());
    }

    void test_wrongHostname()
    {
        TLSCredentials serverCreds;
        QCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        QVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        QCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds, "fucking.hell");

        CFByteArray enc1;
        CFByteArray enc2;
        CFByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        QVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(!server.received(enc1, plain, enc2));
    }

    void test_tls_highSecurity_client()
    {
        TLSCredentials serverCreds;
        QCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        QVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        QCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        QCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds, CFByteArray(), true);

        CFByteArray enc1;
        CFByteArray enc2;
        CFByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        QVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(enc2.isEmpty());

        // send plain to server
        CFByteArray msg = "hello dear server";
        enc1 = CFByteArray();
        client.send(msg, enc1);
        QVERIFY(!enc1.isEmpty());
        QVERIFY(enc1.indexOf(msg) == -1);
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QCOMPARE(plain, msg);
        QVERIFY(enc2.isEmpty());

        // send plain to client
        msg = "hello dear client";
        enc2 = CFByteArray();
        server.send(msg, enc2);
        QVERIFY(!enc2.isEmpty());
        QVERIFY(enc2.indexOf(msg) == -1);
        plain = CFByteArray();
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QCOMPARE(plain, msg);
        QVERIFY(enc1.isEmpty());
    }

    void test_tls_highSecurity_server()
    {
        TLSCredentials serverCreds;
        QCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        QVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds, true);

        TLSCredentials clientCreds;
        QCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        QCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds);

        CFByteArray enc1;
        CFByteArray enc2;
        CFByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        QVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc2.isEmpty());
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QVERIFY(plain.isEmpty());
        QVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QVERIFY(plain.isEmpty());
        QVERIFY(enc2.isEmpty());

        // send plain to server
        CFByteArray msg = "hello dear server";
        enc1 = CFByteArray();
        client.send(msg, enc1);
        QVERIFY(!enc1.isEmpty());
        QVERIFY(enc1.indexOf(msg) == -1);
        enc2 = CFByteArray();
        QVERIFY(server.received(enc1, plain, enc2));
        QCOMPARE(plain, msg);
        QVERIFY(enc2.isEmpty());

        // send plain to client
        msg = "hello dear client";
        enc2 = CFByteArray();
        server.send(msg, enc2);
        QVERIFY(!enc2.isEmpty());
        QVERIFY(enc2.indexOf(msg) == -1);
        plain = CFByteArray();
        enc1 = CFByteArray();
        QVERIFY(client.received(enc2, plain, enc1));
        QCOMPARE(plain, msg);
        QVERIFY(enc1.isEmpty());
    }
};

ADD_TEST(TLSServer_test)
