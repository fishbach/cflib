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

        TVERIFY(creds.getAllCertsPEM().size() == 0);

        TCOMPARE((int)creds.addCerts(CFByteArray()), 0);
        TCOMPARE((int)creds.addCerts(cert3 + cert1), 2);
        TCOMPARE((int)creds.addCerts(cert1), 0);
        TCOMPARE((int)creds.addCerts(cert2), 1);
        TVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));
        TVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey), "wrong"));
        TVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

        const CFList<TLSCertInfo> infos = creds.getCertChainInfos();
        TCOMPARE((int)infos.size(), 3);
        TCOMPARE(infos[0].subjectName, CFByteArray("127.0.0.1"));
        TCOMPARE(infos[0].issuerName,  CFByteArray("ca"));
        TVERIFY(!infos[0].isCA);
        TVERIFY(!infos[0].isTrusted);
        TCOMPARE(infos[1].subjectName, CFByteArray("ca"));
        TCOMPARE(infos[1].issuerName,  CFByteArray("rootca"));
        TVERIFY( infos[1].isCA);
        TVERIFY(!infos[1].isTrusted);
        TCOMPARE(infos[2].subjectName, CFByteArray("rootca"));
        TCOMPARE(infos[2].issuerName,  CFByteArray("rootca"));
        TVERIFY( infos[2].isCA);
        TVERIFY(!infos[2].isTrusted);

        TVERIFY(creds.getAllCertsPEM().size() > 0);
    }

    void test_addCerts_trusted()
    {
        TLSCredentials creds;

        TCOMPARE((int)creds.addCerts(cert2 + cert1), 2);
        TCOMPARE((int)creds.addCerts(cert3, true), 1);
        TVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

        const CFList<TLSCertInfo> infos = creds.getCertChainInfos();
        TCOMPARE((int)infos.size(), 3);
        TCOMPARE(infos[0].subjectName, CFByteArray("127.0.0.1"));
        TVERIFY(!infos[0].isTrusted);
        TCOMPARE(infos[1].subjectName, CFByteArray("ca"));
        TVERIFY(!infos[1].isTrusted);
        TCOMPARE(infos[2].subjectName, CFByteArray("rootca"));
        TVERIFY( infos[2].isTrusted);
    }

    void test_setPrivateKey()
    {
        TLSCredentials creds;

        TVERIFY(!creds.addPrivateKey(CFByteArray()));
        TVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));

        TCOMPARE((int)creds.addCerts(cert2), 1);
        TVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));

        TCOMPARE((int)creds.addCerts(cert1), 1);
        TVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
    }
};

ADD_TEST(TLSCredentials_test)
