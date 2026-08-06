/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/util.h>
#include <cflib/util/test.h>

#include "certs.h"

using namespace cflib::crypt;

TEST_SUITE("TLSCredentials") {

TEST_CASE("TLSCredentials: addCerts")
{
    TLSCredentials creds;

    REQUIRE(creds.getAllCertsPEM().size() == 0);

    REQUIRE_EQ((int)creds.addCerts(ByteArray()), 0);
    REQUIRE_EQ((int)creds.addCerts(cert3 + cert1), 2);
    REQUIRE_EQ((int)creds.addCerts(cert1), 0);
    REQUIRE_EQ((int)creds.addCerts(cert2), 1);
    REQUIRE(!creds.addPrivateKey(detach(cert1PrivateKey)));
    REQUIRE(!creds.addPrivateKey(detach(cert1PrivateKey), "wrong"));
    REQUIRE(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

    const List<TLSCertInfo> infos = creds.getCertChainInfos();
    REQUIRE_EQ((int)infos.size(), 3);
    REQUIRE_EQ(infos[0].subjectName, ByteArray("127.0.0.1"));
    REQUIRE_EQ(infos[0].issuerName,  ByteArray("ca"));
    REQUIRE(!infos[0].isCA);
    REQUIRE(!infos[0].isTrusted);
    REQUIRE_EQ(infos[1].subjectName, ByteArray("ca"));
    REQUIRE_EQ(infos[1].issuerName,  ByteArray("rootca"));
    REQUIRE( infos[1].isCA);
    REQUIRE(!infos[1].isTrusted);
    REQUIRE_EQ(infos[2].subjectName, ByteArray("rootca"));
    REQUIRE_EQ(infos[2].issuerName,  ByteArray("rootca"));
    REQUIRE( infos[2].isCA);
    REQUIRE(!infos[2].isTrusted);

    REQUIRE(creds.getAllCertsPEM().size() > 0);
}

TEST_CASE("TLSCredentials: addCerts_trusted")
{
    TLSCredentials creds;

    REQUIRE_EQ((int)creds.addCerts(cert2 + cert1), 2);
    REQUIRE_EQ((int)creds.addCerts(cert3, true), 1);
    REQUIRE(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

    const List<TLSCertInfo> infos = creds.getCertChainInfos();
    REQUIRE_EQ((int)infos.size(), 3);
    REQUIRE_EQ(infos[0].subjectName, ByteArray("127.0.0.1"));
    REQUIRE(!infos[0].isTrusted);
    REQUIRE_EQ(infos[1].subjectName, ByteArray("ca"));
    REQUIRE(!infos[1].isTrusted);
    REQUIRE_EQ(infos[2].subjectName, ByteArray("rootca"));
    REQUIRE( infos[2].isTrusted);
}

TEST_CASE("TLSCredentials: setPrivateKey")
{
    TLSCredentials creds;

    REQUIRE(!creds.addPrivateKey(ByteArray()));
    REQUIRE(!creds.addPrivateKey(detach(cert1PrivateKey)));

    REQUIRE_EQ((int)creds.addCerts(cert2), 1);
    REQUIRE(!creds.addPrivateKey(detach(cert1PrivateKey)));

    REQUIRE_EQ((int)creds.addCerts(cert1), 1);
    REQUIRE(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
}

}
