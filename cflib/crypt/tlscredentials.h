/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace Botan { class Credentials_Manager; }
namespace Botan { class X509_Certificate; }

namespace cflib { namespace crypt {

struct TLSCertInfo
{
    CFByteArray subjectName;
    CFByteArray issuerName;
    bool isCA;
    bool isTrusted;

    String toString() const;
    bool isNull() const { return subjectName.isEmpty() && issuerName.isEmpty(); }
    TLSCertInfo() : isCA(false), isTrusted(false) {}
};

class TLSCredentials
{
    CF_DISABLE_COPY(TLSCredentials)
public:
    TLSCredentials();
    ~TLSCredentials();

    // certificates can be added in arbitrary order
    uint addCerts(const CFByteArray & certs, bool isTrustedCA = false);
    CFList<TLSCertInfo> getCertChainInfos() const;
    CFList<TLSCertInfo> getAllCertInfos() const;

    uint addRevocationLists(const CFByteArray & crls);

    // private key must be in PKCS8 format
    // fitting certificate must exist
    // builds a certificate chain of added certificates
    // destroys data in parameters
    bool addPrivateKey(const CFByteArray & privateKey, const CFByteArray & password = CFByteArray());

    // Loads all
    //   certificates ending with: _crt.pem
    //   keys         ending with: _key.pem
    //   revocations  ending with: _crl.pem
    // After load activate has to be called.
    // This is necessary when you do a procsess owner change in between.
    // Otherwise the secure memory of botan will not make the move to the new owner.
    bool loadFromDir(const String & path);
    bool activateLoaded(bool isTrustedCA = false);

    // write all certificates to a single .pem file
    CFByteArray getAllCertsPEM() const;

private:
    TLSCertInfo getInfo(const Botan::X509_Certificate & crt) const;

private:
    class Impl;
    Impl * impl_;

    friend class TLSClient;
    friend class TLSServer;
    Botan::Credentials_Manager & credentials_Manager();
};

}}    // namespace
