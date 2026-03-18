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

class TLSCredentials_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<TLSCredentials_test *>(this);
        return {
            {"test_addCerts",         [self]() { self->test_addCerts(); }},
            {"test_addCerts_trusted", [self]() { self->test_addCerts_trusted(); }},
            {"test_setPrivateKey",    [self]() { self->test_setPrivateKey(); }}
        };
    }

    void test_addCerts()
    {
        TLSCredentials creds;

        QVERIFY(creds.getAllCertsPEM().size() == 0);

        QCOMPARE((int)creds.addCerts(CFByteArray()), 0);
        QCOMPARE((int)creds.addCerts(cert3 + cert1), 2);
        QCOMPARE((int)creds.addCerts(cert1), 0);
        QCOMPARE((int)creds.addCerts(cert2), 1);
        QVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));
        QVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey), "wrong"));
        QVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

        const CFList<TLSCertInfo> infos = creds.getCertChainInfos();
        QCOMPARE((int)infos.size(), 3);
        QCOMPARE(infos[0].subjectName, CFByteArray("127.0.0.1"));
        QCOMPARE(infos[0].issuerName,  CFByteArray("ca"));
        QVERIFY(!infos[0].isCA);
        QVERIFY(!infos[0].isTrusted);
        QCOMPARE(infos[1].subjectName, CFByteArray("ca"));
        QCOMPARE(infos[1].issuerName,  CFByteArray("rootca"));
        QVERIFY( infos[1].isCA);
        QVERIFY(!infos[1].isTrusted);
        QCOMPARE(infos[2].subjectName, CFByteArray("rootca"));
        QCOMPARE(infos[2].issuerName,  CFByteArray("rootca"));
        QVERIFY( infos[2].isCA);
        QVERIFY(!infos[2].isTrusted);

        QVERIFY(creds.getAllCertsPEM().size() > 0);
    }

    void test_addCerts_trusted()
    {
        TLSCredentials creds;

        QCOMPARE((int)creds.addCerts(cert2 + cert1), 2);
        QCOMPARE((int)creds.addCerts(cert3, true), 1);
        QVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

        const CFList<TLSCertInfo> infos = creds.getCertChainInfos();
        QCOMPARE((int)infos.size(), 3);
        QCOMPARE(infos[0].subjectName, CFByteArray("127.0.0.1"));
        QVERIFY(!infos[0].isTrusted);
        QCOMPARE(infos[1].subjectName, CFByteArray("ca"));
        QVERIFY(!infos[1].isTrusted);
        QCOMPARE(infos[2].subjectName, CFByteArray("rootca"));
        QVERIFY( infos[2].isTrusted);
    }

    void test_setPrivateKey()
    {
        TLSCredentials creds;

        QVERIFY(!creds.addPrivateKey(CFByteArray()));
        QVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));

        QCOMPARE((int)creds.addCerts(cert2), 1);
        QVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));

        QCOMPARE((int)creds.addCerts(cert1), 1);
        QVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
    }
};

ADD_TEST(TLSCredentials_test)
