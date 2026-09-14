/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "tlscredentials.h"

#include <cflib/crypt/impl/botanhelper.h>
#include <cflib/util/util.h>

#include <dirent.h>

USE_LOG(LogCat::Crypt)

namespace cflib::crypt {

namespace {

ByteArray fromStdVector(const std::vector<std::string> & vec)
{
    if (vec.size() == 0) return ByteArray();
    return ByteArray(vec[0].c_str(), (size_t)vec[0].size());
}

}

String TLSCertInfo::toString() const
{
    String rv("subject: \"");
    rv.append(subjectName.constCharPtr(), subjectName.size());
    rv += "\", issuer: \"";
    rv.append(issuerName.constCharPtr(), issuerName.size());
    rv += "\", isCA: ";
    rv += String::fromInt((int32)isCA);
    rv += ", isTrusted: ";
    rv += String::fromInt((int32)isTrusted);
    return rv;
}

class TLSCredentials::Shared : public Credentials_Manager
{
public:
    std::vector<Certificate_Store *> trusted_certificate_authorities(const std::string & type, const std::string& context) override
    {
        CF_UNUSED(type);
        CF_UNUSED(context);
        std::vector<Certificate_Store *> rv(1);
        rv[0] = &trustedCAs;
        return rv;
    }

    std::vector<X509_Certificate> cert_chain(const std::vector<std::string> & cert_key_types,
        const std::vector<AlgorithmIdentifier> & cert_signature_schemes,
        const std::string & type, const std::string & context) override
    {
        CF_UNUSED(cert_signature_schemes);
        if (type != "tls-server") return std::vector<X509_Certificate>();
        for (const CertsPrivKey & ck : chains) {
            if (context != "" && !ck.certs[0].matches_dns_name(context)) continue;
            for (const std::string & kt : cert_key_types) {
                if (kt == ck.privateKey->algo_name()) return ck.certs;
            }
        }
        return std::vector<X509_Certificate>();
    }

    std::shared_ptr<Private_Key> private_key_for(const X509_Certificate & cert, const std::string &, const std::string &) override
    {
        for (const CertsPrivKey & ck : chains) if (ck.certs[0] == cert) return ck.privateKey;
        return std::shared_ptr<Private_Key>();
    }

public:
    Shared() = default;
    Shared(const Shared & other) :
        chains     (other.chains),
        allCerts   (other.allCerts),
        trustedCAs (other.trustedCAs),
        loadedCerts(other.loadedCerts),
        loadedKeys (other.loadedKeys),
        loadedCrls (other.loadedCrls)
    {}

public:
    AtomicInt ref = 1;
    struct CertsPrivKey {
        std::vector<X509_Certificate> certs;
        std::shared_ptr<Private_Key> privateKey;

        CertsPrivKey() = default;
        CertsPrivKey(const CertsPrivKey & other) { operator=(other); }
        CertsPrivKey & operator=(const CertsPrivKey & other) {
            certs      = other.certs;
            privateKey = PKCS8::copy_key(*other.privateKey);
            return *this;
        }
    };
    List<CertsPrivKey> chains;
    List<X509_Certificate> allCerts;
    Certificate_Store_In_Memory trustedCAs;
    ByteArrayList loadedCerts;
    ByteArrayList loadedKeys;
    ByteArrayList loadedCrls;
};

TLSCredentials::TLSCredentials() :
    d(new Shared)
{
}

TLSCredentials::~TLSCredentials()
{
    if (!d->ref.deref()) delete d;
}

TLSCredentials::TLSCredentials(const TLSCredentials & other) :
    d(other.d)
{
    d->ref.ref();
}

TLSCredentials::TLSCredentials(TLSCredentials && other) :
    d(other.d)
{
    other.d = new Shared;
}

TLSCredentials & TLSCredentials::operator=(const TLSCredentials & other)
{
    if (d == other.d) return *this;
    if (!d->ref.deref()) delete d;
    d = other.d;
    d->ref.ref();
    return *this;
}

TLSCredentials & TLSCredentials::operator=(TLSCredentials && other)
{
    std::swap(d, other.d);
    return *this;
}

void TLSCredentials::detach()
{
    if (d->ref.loadAcquire() == 1) return;
    Shared * newD = new Shared(*d);
    if (!d->ref.deref()) delete d;
    d = newD;
}

bool TLSCredentials::isEmpty() const
{
    return d->allCerts.isEmpty();
}

uint TLSCredentials::addCerts(const ByteArray & certs, bool isTrustedCA)
{
    detach();

    uint rv = 0;
    try {
        DataSource_Memory ds((const byte *)certs.constCharPtr(), certs.size());
        while (true) {
            X509_Certificate crt(ds);
            bool found = false;
            for (const X509_Certificate & existing : d->allCerts) {
                if (existing == crt) { found = true; break; }
            }
            if (found) continue;
            d->allCerts.push_back(crt);
            if (isTrustedCA) d->trustedCAs.add_certificate(crt);
            ++rv;
        }
    } catch (...) {}
    return rv;
}

uint TLSCredentials::addRevocationLists(const ByteArray & crls)
{
    detach();

    uint rv = 0;
    try {
        DataSource_Memory ds((const byte *)crls.constCharPtr(), crls.size());
        while (true) {
            X509_CRL crl(ds);
            d->trustedCAs.add_crl(crl);
            ++rv;
        }
    } catch (...) {}
    return rv;
}

bool TLSCredentials::addPrivateKey(const ByteArray & privateKey, const ByteArray & password)
{
    detach();

    TRY {
        DataSource_Memory ds((const byte *)privateKey.constCharPtr(), privateKey.size());
        std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds, std::string(password.constCharPtr(), password.size())));

        // destroy data in parameters
        for (size_t i = 0 ; i < privateKey.size() ; ++i) ((char *)privateKey.constCharPtr())[i] = 0;
        for (size_t i = 0 ; i < password.size()   ; ++i) ((char *)password  .constCharPtr())[i] = 0;

        if (!pk) return false;

        // Does key exist?
        const std::vector<byte> pubKey = pk->subject_public_key();
        for (const Shared::CertsPrivKey & ck : d->chains) {
            if (pubKey == ck.privateKey->subject_public_key()) return false;
        }

        // search cert
        std::vector<X509_Certificate> certs;
        for (const X509_Certificate & crt : d->allCerts) {
            std::unique_ptr<Public_Key> certPubKey(crt.subject_public_key());
            if (pubKey == certPubKey->subject_public_key()) {
                certs.push_back(crt);
                break;
            }
        }
        if (certs.size() == 0) return false;

        // build chain
        again:
        const std::vector<byte> authorityKeyId = certs[certs.size() - 1].authority_key_id();
        for (const X509_Certificate & crt : d->allCerts) {
            if (crt.subject_key_id() == authorityKeyId &&
                std::find(certs.begin(), certs.end(), crt) == certs.end())
            {
                certs.push_back(crt);
                goto again;
            }
        }

        Shared::CertsPrivKey ck;
        ck.certs = certs;
        ck.privateKey = std::move(pk);
        d->chains.push_back(ck);

        return true;
    } CATCH
    return false;
}

bool TLSCredentials::loadFromDir(const String & path)
{
    detach();

    DIR * dir = opendir(path.toStdString().c_str());
    if (!dir) return false;
    struct dirent * entry;
    while ((entry = readdir(dir)) != nullptr) {
        String name(entry->d_name);
        String file = path + "/" + name;
        if (name.endsWith("_crt.pem")) {
            ByteArray data = File::read(file);
            if (data.isEmpty()) {
                logWarn("could not read certificate: %1", file);
                continue;
            }
            d->loadedCerts.push_back(data);
        } else if (name.endsWith("_key.pem")) {
            ByteArray data = File::read(file);
            if (data.isEmpty()) {
                logWarn("could not read key: %1", file);
                continue;
            }
            d->loadedKeys.push_back(data);
        } else if (name.endsWith("_crl.pem")) {
            ByteArray data = File::read(file);
            if (data.isEmpty()) {
                logWarn("could not read revocation list: %1", file);
                continue;
            }
            d->loadedCrls.push_back(data);
        }
    }
    closedir(dir);
    return true;
}

bool TLSCredentials::activateLoaded(bool isTrustedCA)
{
    detach();

    bool ok = true;
    for (const ByteArray & data : d->loadedCerts) {
        if (addCerts(data, isTrustedCA) == 0) {
            logWarn("could not handle certificate: %1", data);
            ok = false;
        }
    }
    d->loadedCerts.clear();
    for (const ByteArray & data : d->loadedKeys) {
        if (!addPrivateKey(data)) {
            logWarn("could not handle key: %1", data.left(40));
            ok = false;
        }
    }
    d->loadedKeys.clear();
    for (const ByteArray & data : d->loadedCrls) {
        if (addRevocationLists(data) == 0) {
            logWarn("could not handle revocation list: %1", data);
            ok = false;
        }
    }
    d->loadedCrls.clear();

    const List<TLSCertInfo> infos = getAllCertInfos();
    logInfo("loaded %1 certifices:", (uint64)infos.size());
    for (const TLSCertInfo & info : infos) {
        logInfo("  cert: %1", info);
    }

    return ok;
}

TLSCertInfo TLSCredentials::getInfo(const X509_Certificate & crt) const
{
    TRY {
        TLSCertInfo info;
        info.subjectName = fromStdVector(crt.subject_dn().get_attribute("X520.CommonName"));
        info.issuerName  = fromStdVector(crt.issuer_dn ().get_attribute("X520.CommonName"));
        info.isCA        = crt.is_CA_cert();
        for (Certificate_Store * cs : d->trusted_certificate_authorities("", "")) {
            if (cs->find_cert(crt.subject_dn(), crt.subject_key_id())) {
                info.isTrusted = true;
                break;
            }
        }
        return info;
    } CATCH
    return TLSCertInfo();
}

List<TLSCertInfo> TLSCredentials::getCertChainInfos() const
{
    List<TLSCertInfo> rv;
    for (const Shared::CertsPrivKey & ck : d->chains) {
        for (const X509_Certificate & crt : ck.certs) {
            TLSCertInfo info = getInfo(crt);
            if (!info.isNull()) rv.push_back(info);
        }
    }
    return rv;
}

List<TLSCertInfo> TLSCredentials::getAllCertInfos() const
{
    List<TLSCertInfo> rv;
    for (const X509_Certificate & crt : d->allCerts) {
        TLSCertInfo info = getInfo(crt);
        if (!info.isNull()) rv.push_back(info);
    }
    return rv;
}

ByteArray TLSCredentials::getAllCertsPEM() const
{
    TRY {
        ByteArray rv("");    // not null
        for (const X509_Certificate & cert : d->allCerts) {
            DER_Encoder enc;
            cert.encode_into(enc);
            const std::string pem = PEM_Code::encode(enc.get_contents(), "CERTIFICATE");
            rv += ByteArray(pem.c_str(), (size_t)pem.size());
            rv += '\n';
        }
        return rv;
    } CATCH
    return ByteArray();
}

Botan::Credentials_Manager & TLSCredentials::credentials_Manager()
{
    // We do not detach here, as the Credentials_Manager interface is const,
    // even though it does not look like that.
    return *d;
}

} // namespace
