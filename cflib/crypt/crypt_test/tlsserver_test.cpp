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
        TCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        TVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        TCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        TCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds);

        ByteArray enc1;
        ByteArray enc2;
        ByteArray plain;

        // client starts handshake
        TVERIFY(server.initialSend().isEmpty());
        enc1 = client.initialSend();
        TVERIFY(!enc1.isEmpty());
        TVERIFY(client.initialSend().isEmpty());

        // first handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(enc2.isEmpty());

        // send plain to server
        ByteArray msg = "hello dear server";
        enc1 = ByteArray();
        client.send(msg, enc1);
        TVERIFY(!enc1.isEmpty());
        TVERIFY(enc1.indexOf(msg) == -1);
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TCOMPARE(plain, msg);
        TVERIFY(enc2.isEmpty());

        // send plain to client
        msg = "hello dear client";
        enc2 = ByteArray();
        server.send(msg, enc2);
        TVERIFY(!enc2.isEmpty());
        TVERIFY(enc2.indexOf(msg) == -1);
        plain = ByteArray();
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TCOMPARE(plain, msg);
        TVERIFY(enc1.isEmpty());
    }

    void test_fastSend()
    {
        TLSCredentials serverCreds;
        TCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        TVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        TCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        TCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds);

        ByteArray enc1;
        ByteArray enc2;
        ByteArray plain;

        // send before handshake
        ByteArray serverMsg = "hello dear client";
        enc2 = ByteArray();
        TVERIFY(server.send(serverMsg, enc2));
        TVERIFY(enc2.isEmpty());

        // send before handshake
        ByteArray clientMsg = "hello dear server";
        enc1 = ByteArray();
        TVERIFY(client.send(clientMsg, enc1));
        TVERIFY(enc1.isEmpty());

        // client starts handshake
        enc1 = client.initialSend();
        TVERIFY(!enc1.isEmpty());
        TVERIFY(enc1.indexOf(clientMsg) == -1);

        // first handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TCOMPARE(plain, clientMsg);
        plain = ByteArray();
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TCOMPARE(plain, serverMsg);
        TVERIFY(enc1.isEmpty());
    }

    void test_missingCA()
    {
        TLSCredentials serverCreds;
        TCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        TVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds);

        ByteArray enc1;
        ByteArray enc2;
        ByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        TVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(!client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
    }

    void test_hostname()
    {
        TLSCredentials serverCreds;
        TCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        TVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        TCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        TCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds, "127.0.0.1");

        ByteArray enc1;
        ByteArray enc2;
        ByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        TVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(enc2.isEmpty());

        // send plain to server
        ByteArray msg = "hello dear server";
        enc1 = ByteArray();
        TVERIFY(client.send(msg, enc1));
        TVERIFY(!enc1.isEmpty());
        TVERIFY(enc1.indexOf(msg) == -1);
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TCOMPARE(plain, msg);
        TVERIFY(enc2.isEmpty());

        // send plain to client
        msg = "hello dear client";
        enc2 = ByteArray();
        TVERIFY(server.send(msg, enc2));
        TVERIFY(!enc2.isEmpty());
        TVERIFY(enc2.indexOf(msg) == -1);
        plain = ByteArray();
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TCOMPARE(plain, msg);
        TVERIFY(enc1.isEmpty());
    }

    void test_wrongHostname()
    {
        TLSCredentials serverCreds;
        TCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        TVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        TCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds, "fucking.hell");

        ByteArray enc1;
        ByteArray enc2;
        ByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        TVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(!server.received(enc1, plain, enc2));
    }

    void test_tls_highSecurity_client()
    {
        TLSCredentials serverCreds;
        TCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        TVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds);

        TLSCredentials clientCreds;
        TCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        TCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds, ByteArray(), true);

        ByteArray enc1;
        ByteArray enc2;
        ByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        TVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(enc2.isEmpty());

        // send plain to server
        ByteArray msg = "hello dear server";
        enc1 = ByteArray();
        client.send(msg, enc1);
        TVERIFY(!enc1.isEmpty());
        TVERIFY(enc1.indexOf(msg) == -1);
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TCOMPARE(plain, msg);
        TVERIFY(enc2.isEmpty());

        // send plain to client
        msg = "hello dear client";
        enc2 = ByteArray();
        server.send(msg, enc2);
        TVERIFY(!enc2.isEmpty());
        TVERIFY(enc2.indexOf(msg) == -1);
        plain = ByteArray();
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TCOMPARE(plain, msg);
        TVERIFY(enc1.isEmpty());
    }

    void test_tls_highSecurity_server()
    {
        TLSCredentials serverCreds;
        TCOMPARE((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
        TVERIFY(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
        TLSSessions serverSessions;
        TLSServer server(serverSessions, serverCreds, true);

        TLSCredentials clientCreds;
        TCOMPARE((int)clientCreds.addCerts(cert3, true), 1);
        TCOMPARE((int)clientCreds.addRevocationLists(cert2Crl), 1);
        TLSSessions clientSessions;
        TLSClient client(clientSessions, clientCreds);

        ByteArray enc1;
        ByteArray enc2;
        ByteArray plain;

        // client starts handshake
        enc1 = client.initialSend();
        TVERIFY(!enc1.isEmpty());

        // first handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // second handshake req -> reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc2.isEmpty());
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TVERIFY(plain.isEmpty());
        TVERIFY(!enc1.isEmpty());

        // third handshake req, no reply
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TVERIFY(plain.isEmpty());
        TVERIFY(enc2.isEmpty());

        // send plain to server
        ByteArray msg = "hello dear server";
        enc1 = ByteArray();
        client.send(msg, enc1);
        TVERIFY(!enc1.isEmpty());
        TVERIFY(enc1.indexOf(msg) == -1);
        enc2 = ByteArray();
        TVERIFY(server.received(enc1, plain, enc2));
        TCOMPARE(plain, msg);
        TVERIFY(enc2.isEmpty());

        // send plain to client
        msg = "hello dear client";
        enc2 = ByteArray();
        server.send(msg, enc2);
        TVERIFY(!enc2.isEmpty());
        TVERIFY(enc2.indexOf(msg) == -1);
        plain = ByteArray();
        enc1 = ByteArray();
        TVERIFY(client.received(enc2, plain, enc1));
        TCOMPARE(plain, msg);
        TVERIFY(enc1.isEmpty());
    }
};

ADD_TEST(TLSServer_test)
