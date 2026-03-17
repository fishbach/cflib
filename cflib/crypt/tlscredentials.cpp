/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
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

namespace cflib { namespace crypt {

namespace {

CFByteArray fromStdVector(const std::vector<std::string> & vec)
{
    if (vec.size() == 0) return CFByteArray();
    return CFByteArray(vec[0].c_str(), (cfsize_t)vec[0].size());
}

}

CFString TLSCertInfo::toString() const
{
    CFString rv("subject: \"");
    rv += CFString(subjectName.constData());
    rv += "\", issuer: \"";
    rv += CFString(issuerName.constData());
    rv += "\", isCA: ";
    rv += CFString::number((cfint32)isCA);
    rv += ", isTrusted: ";
    rv += CFString::number((cfint32)isTrusted);
    return rv;
}

class TLSCredentials::Impl : public Credentials_Manager
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
    struct CertsPrivKey {
        std::vector<X509_Certificate> certs;
        std::shared_ptr<Private_Key> privateKey;

        CertsPrivKey() : privateKey(0) {}
    };
    CFList<CertsPrivKey> chains;
    CFList<X509_Certificate> allCerts;
    Certificate_Store_In_Memory trustedCAs;
    CFList<CFByteArray> loadedCerts;
    CFList<CFByteArray> loadedKeys;
    CFList<CFByteArray> loadedCrls;
};

TLSCredentials::TLSCredentials() :
    impl_(new Impl)
{
}

TLSCredentials::~TLSCredentials()
{
    delete impl_;
}

uint TLSCredentials::addCerts(const CFByteArray & certs, bool isTrustedCA)
{
    uint rv = 0;
    try {
        DataSource_Memory ds((const byte *)certs.constData(), certs.size());
        while (true) {
            X509_Certificate crt(ds);
            bool found = false;
            for (const X509_Certificate & existing : impl_->allCerts) {
                if (existing == crt) { found = true; break; }
            }
            if (found) continue;
            impl_->allCerts.push_back(crt);
            if (isTrustedCA) impl_->trustedCAs.add_certificate(crt);
            ++rv;
        }
    } catch (...) {}
    return rv;
}

uint TLSCredentials::addRevocationLists(const CFByteArray & crls)
{
    uint rv = 0;
    try {
        DataSource_Memory ds((const byte *)crls.constData(), crls.size());
        while (true) {
            X509_CRL crl(ds);
            impl_->trustedCAs.add_crl(crl);
            ++rv;
        }
    } catch (...) {}
    return rv;
}

bool TLSCredentials::addPrivateKey(const CFByteArray & privateKey, const CFByteArray & password)
{
    TRY {
        DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
        std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds, std::string(password.constData(), password.size())));

        // destroy data in parameters
        for (cfsize_t i = 0 ; i < privateKey.size() ; ++i) ((char *)privateKey.constData())[i] = 0;
        for (cfsize_t i = 0 ; i < password.size()   ; ++i) ((char *)password  .constData())[i] = 0;

        if (!pk) return false;

        // Does key exist?
        const std::vector<byte> pubKey = pk->subject_public_key();
        for (const Impl::CertsPrivKey & ck : impl_->chains) {
            if (pubKey == ck.privateKey->subject_public_key()) return false;
        }

        // search cert
        std::vector<X509_Certificate> certs;
        for (const X509_Certificate & crt : impl_->allCerts) {
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
        for (const X509_Certificate & crt : impl_->allCerts) {
            if (crt.subject_key_id() == authorityKeyId &&
                std::find(certs.begin(), certs.end(), crt) == certs.end())
            {
                certs.push_back(crt);
                goto again;
            }
        }

        Impl::CertsPrivKey ck;
        ck.certs = certs;
        ck.privateKey = std::move(pk);
        impl_->chains.push_back(ck);

        return true;
    } CATCH
    return false;
}

bool TLSCredentials::loadFromDir(const CFString & path)
{
    DIR * dir = opendir(path.c_str());
    if (!dir) return false;
    struct dirent * entry;
    while ((entry = readdir(dir)) != nullptr) {
        CFString name(entry->d_name);
        CFString file = CFString(path.str() + "/" + name.str());
        if (name.endsWith("_crt.pem")) {
            CFByteArray data = util::readFile(file);
            if (data.isEmpty()) {
                logWarn("could not read certificate: %1", file);
                continue;
            }
            impl_->loadedCerts.push_back(data);
        } else if (name.endsWith("_key.pem")) {
            CFByteArray data = util::readFile(file);
            if (data.isEmpty()) {
                logWarn("could not read key: %1", file);
                continue;
            }
            impl_->loadedKeys.push_back(data);
        } else if (name.endsWith("_crl.pem")) {
            CFByteArray data = util::readFile(file);
            if (data.isEmpty()) {
                logWarn("could not read revocation list: %1", file);
                continue;
            }
            impl_->loadedCrls.push_back(data);
        }
    }
    closedir(dir);
    return true;
}

bool TLSCredentials::activateLoaded(bool isTrustedCA)
{
    bool ok = true;
    for (const CFByteArray & data : impl_->loadedCerts) {
        if (addCerts(data, isTrustedCA) == 0) {
            logWarn("could not handle certificate: %1", data);
            ok = false;
        }
    }
    impl_->loadedCerts.clear();
    for (const CFByteArray & data : impl_->loadedKeys) {
        if (!addPrivateKey(data)) {
            logWarn("could not handle key: %1", data.left(40));
            ok = false;
        }
    }
    impl_->loadedKeys.clear();
    for (const CFByteArray & data : impl_->loadedCrls) {
        if (addRevocationLists(data) == 0) {
            logWarn("could not handle revocation list: %1", data);
            ok = false;
        }
    }
    impl_->loadedCrls.clear();

    const CFList<TLSCertInfo> infos = getAllCertInfos();
    logInfo("loaded %1 certifices:", (cfuint64)infos.size());
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
        for (Certificate_Store * cs : impl_->trusted_certificate_authorities("", "")) {
            if (cs->find_cert(crt.subject_dn(), crt.subject_key_id())) {
                info.isTrusted = true;
                break;
            }
        }
        return info;
    } CATCH
    return TLSCertInfo();
}

CFList<TLSCertInfo> TLSCredentials::getCertChainInfos() const
{
    CFList<TLSCertInfo> rv;
    for (const Impl::CertsPrivKey & ck : impl_->chains) {
        for (const X509_Certificate & crt : ck.certs) {
            TLSCertInfo info = getInfo(crt);
            if (!info.isNull()) rv.push_back(info);
        }
    }
    return rv;
}

CFList<TLSCertInfo> TLSCredentials::getAllCertInfos() const
{
    CFList<TLSCertInfo> rv;
    for (const X509_Certificate & crt : impl_->allCerts) {
        TLSCertInfo info = getInfo(crt);
        if (!info.isNull()) rv.push_back(info);
    }
    return rv;
}

CFByteArray TLSCredentials::getAllCertsPEM() const
{
    TRY {
        CFByteArray rv("");    // not null
        for (const X509_Certificate & cert : impl_->allCerts) {
            DER_Encoder enc;
            cert.encode_into(enc);
            const std::string pem = PEM_Code::encode(enc.get_contents(), "CERTIFICATE");
            rv += CFByteArray(pem.c_str(), (cfsize_t)pem.size());
            rv += '\n';
        }
        return rv;
    } CATCH
    return CFByteArray();
}

Botan::Credentials_Manager & TLSCredentials::credentials_Manager()
{
    return *impl_;
}

}}    // namespace
