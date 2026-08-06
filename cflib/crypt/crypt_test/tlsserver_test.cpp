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

TEST_SUITE("TLSServer") {

TEST_CASE("TLSServer: tls")
{
    TLSCredentials serverCreds;
    REQUIRE_EQ((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
    REQUIRE(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
    TLSSessions serverSessions;
    TLSServer server(serverSessions, serverCreds);

    TLSCredentials clientCreds;
    REQUIRE_EQ((int)clientCreds.addCerts(cert3, true), 1);
    REQUIRE_EQ((int)clientCreds.addRevocationLists(cert2Crl), 1);
    TLSSessions clientSessions;
    TLSClient client(clientSessions, clientCreds);

    ByteArray enc1;
    ByteArray enc2;
    ByteArray plain;

    // client starts handshake
    REQUIRE(server.initialSend().isEmpty());
    enc1 = client.initialSend();
    REQUIRE(!enc1.isEmpty());
    REQUIRE(client.initialSend().isEmpty());

    // first handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // second handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // third handshake req, no reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(enc2.isEmpty());

    // send plain to server
    ByteArray msg = "hello dear server";
    enc1 = ByteArray();
    client.send(msg, enc1);
    REQUIRE(!enc1.isEmpty());
    REQUIRE(enc1.indexOf(msg) == -1);
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE_EQ(plain, msg);
    REQUIRE(enc2.isEmpty());

    // send plain to client
    msg = "hello dear client";
    enc2 = ByteArray();
    server.send(msg, enc2);
    REQUIRE(!enc2.isEmpty());
    REQUIRE(enc2.indexOf(msg) == -1);
    plain = ByteArray();
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE_EQ(plain, msg);
    REQUIRE(enc1.isEmpty());
}

TEST_CASE("TLSServer: fastSend")
{
    TLSCredentials serverCreds;
    REQUIRE_EQ((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
    REQUIRE(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
    TLSSessions serverSessions;
    TLSServer server(serverSessions, serverCreds);

    TLSCredentials clientCreds;
    REQUIRE_EQ((int)clientCreds.addCerts(cert3, true), 1);
    REQUIRE_EQ((int)clientCreds.addRevocationLists(cert2Crl), 1);
    TLSSessions clientSessions;
    TLSClient client(clientSessions, clientCreds);

    ByteArray enc1;
    ByteArray enc2;
    ByteArray plain;

    // send before handshake
    ByteArray serverMsg = "hello dear client";
    enc2 = ByteArray();
    REQUIRE(server.send(serverMsg, enc2));
    REQUIRE(enc2.isEmpty());

    // send before handshake
    ByteArray clientMsg = "hello dear server";
    enc1 = ByteArray();
    REQUIRE(client.send(clientMsg, enc1));
    REQUIRE(enc1.isEmpty());

    // client starts handshake
    enc1 = client.initialSend();
    REQUIRE(!enc1.isEmpty());
    REQUIRE(enc1.indexOf(clientMsg) == -1);

    // first handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // second handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // third handshake req, no reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE_EQ(plain, clientMsg);
    plain = ByteArray();
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE_EQ(plain, serverMsg);
    REQUIRE(enc1.isEmpty());
}

TEST_CASE("TLSServer: missingCA")
{
    TLSCredentials serverCreds;
    REQUIRE_EQ((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
    REQUIRE(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
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
    REQUIRE(!enc1.isEmpty());

    // first handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // second handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(!client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
}

TEST_CASE("TLSServer: hostname")
{
    TLSCredentials serverCreds;
    REQUIRE_EQ((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
    REQUIRE(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
    TLSSessions serverSessions;
    TLSServer server(serverSessions, serverCreds);

    TLSCredentials clientCreds;
    REQUIRE_EQ((int)clientCreds.addCerts(cert3, true), 1);
    REQUIRE_EQ((int)clientCreds.addRevocationLists(cert2Crl), 1);
    TLSSessions clientSessions;
    TLSClient client(clientSessions, clientCreds, "127.0.0.1");

    ByteArray enc1;
    ByteArray enc2;
    ByteArray plain;

    // client starts handshake
    enc1 = client.initialSend();
    REQUIRE(!enc1.isEmpty());

    // first handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // second handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // third handshake req, no reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(enc2.isEmpty());

    // send plain to server
    ByteArray msg = "hello dear server";
    enc1 = ByteArray();
    REQUIRE(client.send(msg, enc1));
    REQUIRE(!enc1.isEmpty());
    REQUIRE(enc1.indexOf(msg) == -1);
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE_EQ(plain, msg);
    REQUIRE(enc2.isEmpty());

    // send plain to client
    msg = "hello dear client";
    enc2 = ByteArray();
    REQUIRE(server.send(msg, enc2));
    REQUIRE(!enc2.isEmpty());
    REQUIRE(enc2.indexOf(msg) == -1);
    plain = ByteArray();
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE_EQ(plain, msg);
    REQUIRE(enc1.isEmpty());
}

TEST_CASE("TLSServer: wrongHostname")
{
    TLSCredentials serverCreds;
    REQUIRE_EQ((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
    REQUIRE(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
    TLSSessions serverSessions;
    TLSServer server(serverSessions, serverCreds);

    TLSCredentials clientCreds;
    REQUIRE_EQ((int)clientCreds.addCerts(cert3, true), 1);
    TLSSessions clientSessions;
    TLSClient client(clientSessions, clientCreds, "fucking.hell");

    ByteArray enc1;
    ByteArray enc2;
    ByteArray plain;

    // client starts handshake
    enc1 = client.initialSend();
    REQUIRE(!enc1.isEmpty());

    // first handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // second handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(!server.received(enc1, plain, enc2));
}

TEST_CASE("TLSServer: tls_highSecurity_client")
{
    TLSCredentials serverCreds;
    REQUIRE_EQ((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
    REQUIRE(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
    TLSSessions serverSessions;
    TLSServer server(serverSessions, serverCreds);

    TLSCredentials clientCreds;
    REQUIRE_EQ((int)clientCreds.addCerts(cert3, true), 1);
    REQUIRE_EQ((int)clientCreds.addRevocationLists(cert2Crl), 1);
    TLSSessions clientSessions;
    TLSClient client(clientSessions, clientCreds, ByteArray(), true);

    ByteArray enc1;
    ByteArray enc2;
    ByteArray plain;

    // client starts handshake
    enc1 = client.initialSend();
    REQUIRE(!enc1.isEmpty());

    // first handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // second handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // third handshake req, no reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(enc2.isEmpty());

    // send plain to server
    ByteArray msg = "hello dear server";
    enc1 = ByteArray();
    client.send(msg, enc1);
    REQUIRE(!enc1.isEmpty());
    REQUIRE(enc1.indexOf(msg) == -1);
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE_EQ(plain, msg);
    REQUIRE(enc2.isEmpty());

    // send plain to client
    msg = "hello dear client";
    enc2 = ByteArray();
    server.send(msg, enc2);
    REQUIRE(!enc2.isEmpty());
    REQUIRE(enc2.indexOf(msg) == -1);
    plain = ByteArray();
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE_EQ(plain, msg);
    REQUIRE(enc1.isEmpty());
}

TEST_CASE("TLSServer: tls_highSecurity_server")
{
    TLSCredentials serverCreds;
    REQUIRE_EQ((int)serverCreds.addCerts(cert1 + cert2 + cert3), 3);
    REQUIRE(serverCreds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
    TLSSessions serverSessions;
    TLSServer server(serverSessions, serverCreds, true);

    TLSCredentials clientCreds;
    REQUIRE_EQ((int)clientCreds.addCerts(cert3, true), 1);
    REQUIRE_EQ((int)clientCreds.addRevocationLists(cert2Crl), 1);
    TLSSessions clientSessions;
    TLSClient client(clientSessions, clientCreds);

    ByteArray enc1;
    ByteArray enc2;
    ByteArray plain;

    // client starts handshake
    enc1 = client.initialSend();
    REQUIRE(!enc1.isEmpty());

    // first handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // second handshake req -> reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc2.isEmpty());
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE(plain.isEmpty());
    REQUIRE(!enc1.isEmpty());

    // third handshake req, no reply
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE(plain.isEmpty());
    REQUIRE(enc2.isEmpty());

    // send plain to server
    ByteArray msg = "hello dear server";
    enc1 = ByteArray();
    client.send(msg, enc1);
    REQUIRE(!enc1.isEmpty());
    REQUIRE(enc1.indexOf(msg) == -1);
    enc2 = ByteArray();
    REQUIRE(server.received(enc1, plain, enc2));
    REQUIRE_EQ(plain, msg);
    REQUIRE(enc2.isEmpty());

    // send plain to client
    msg = "hello dear client";
    enc2 = ByteArray();
    server.send(msg, enc2);
    REQUIRE(!enc2.isEmpty());
    REQUIRE(enc2.indexOf(msg) == -1);
    plain = ByteArray();
    enc1 = ByteArray();
    REQUIRE(client.received(enc2, plain, enc1));
    REQUIRE_EQ(plain, msg);
    REQUIRE(enc1.isEmpty());
}

}
