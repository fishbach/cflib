/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "tlscredentials.h"

#include <cflib/crypt/impl/botanhelper.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Crypt)

namespace cflib { namespace crypt {

namespace {

QByteArray fromStdVector(const std::vector<std::string> & vec)
{
    if (vec.size() == 0) return QByteArray();
    return QByteArray::fromStdString(vec[0]);
}

}

QString TLSCertInfo::toString() const
{
    return QString("subject: \"%1\", issuer: \"%2\", isCA: %3, isTrusted: %4")
        .arg(QString::fromUtf8(subjectName)).arg(QString::fromUtf8(issuerName)).arg(isCA).arg(isTrusted);
}

class TLSCredentials::Impl : public Credentials_Manager
{
public:
    std::vector<Certificate_Store *> trusted_certificate_authorities(const std::string & type, const std::string& context) override
    {
        Q_UNUSED(type)
        Q_UNUSED(context)
        std::vector<Certificate_Store *> rv(1);
        rv[0] = &trustedCAs;
        return rv;
    }

    std::vector<X509_Certificate> cert_chain(const std::vector<std::string> & cert_key_types,
        const std::vector<AlgorithmIdentifier> & cert_signature_schemes,
        const std::string & type, const std::string & context) override
    {
        Q_UNUSED(cert_signature_schemes)
        if (type != "tls-server") return std::vector<X509_Certificate>();
        foreach (const CertsPrivKey & ck, chains) {
            if (context != "" && !ck.certs[0].matches_dns_name(context)) continue;
            foreach (const std::string & kt, cert_key_types) {
                if (kt == ck.privateKey->algo_name()) return ck.certs;
            }
        }
        return std::vector<X509_Certificate>();
    }

    std::shared_ptr<Private_Key> private_key_for(const X509_Certificate & cert, const std::string &, const std::string &) override
    {
        foreach (const CertsPrivKey & ck, chains) if (ck.certs[0] == cert) return ck.privateKey;
        return std::shared_ptr<Private_Key>();
    }

public:
    struct CertsPrivKey {
        std::vector<X509_Certificate> certs;
        std::shared_ptr<Private_Key> privateKey;

        CertsPrivKey() : privateKey(0) {}
    };
    QList<CertsPrivKey> chains;
    QList<X509_Certificate> allCerts;
    Certificate_Store_In_Memory trustedCAs;
    QList<QByteArray> loadedCerts;
    QList<QByteArray> loadedKeys;
    QList<QByteArray> loadedCrls;
};

TLSCredentials::TLSCredentials() :
    impl_(new Impl)
{
}

TLSCredentials::~TLSCredentials()
{
    delete impl_;
}

uint TLSCredentials::addCerts(const QByteArray & certs, bool isTrustedCA)
{
    uint rv = 0;
    try {
        DataSource_Memory ds((const byte *)certs.constData(), certs.size());
        forever {
            X509_Certificate crt(ds);
            if (impl_->allCerts.contains(crt)) continue;
            impl_->allCerts << crt;
            if (isTrustedCA) impl_->trustedCAs.add_certificate(crt);
            ++rv;
        }
    } catch (...) {}
    return rv;
}

uint TLSCredentials::addRevocationLists(const QByteArray & crls)
{
    uint rv = 0;
    try {
        DataSource_Memory ds((const byte *)crls.constData(), crls.size());
        forever {
            X509_CRL crl(ds);
            impl_->trustedCAs.add_crl(crl);
            ++rv;
        }
    } catch (...) {}
    return rv;
}

bool TLSCredentials::addPrivateKey(const QByteArray & privateKey, const QByteArray & password)
{
    TRY {
        DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
        std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds, password.toStdString()));

        // destroy data in parameters
        for (int i = 0 ; i < privateKey.size() ; ++i) ((char *)privateKey.constData())[i] = 0;
        for (int i = 0 ; i < password.size()   ; ++i) ((char *)password  .constData())[i] = 0;

        if (!pk) return false;

        // Does key exist?
        const std::vector<byte> pubKey = pk->subject_public_key();
        foreach (const Impl::CertsPrivKey & ck, impl_->chains) {
            if (pubKey == ck.privateKey->subject_public_key()) return false;
        }

        // search cert
        std::vector<X509_Certificate> certs;
        foreach (const X509_Certificate & crt, impl_->allCerts) {
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
        foreach (const X509_Certificate & crt, impl_->allCerts) {
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
        impl_->chains << ck;

        return true;
    } CATCH
    return false;
}

bool TLSCredentials::loadFromDir(const QString & path)
{
    QDir dir(path);
    foreach (const QFileInfo & fi, dir.entryInfoList(QStringList() << "*_crt.pem", QDir::Readable | QDir::Files)) {
        const QString file = fi.absoluteFilePath();
        const QByteArray data = util::readFile(file);
        if (data.isEmpty()) {
            logWarn("could not read certificate: %1", file);
            continue;
        }
        impl_->loadedCerts << data;
    }
    foreach (const QFileInfo & fi, dir.entryInfoList(QStringList() << "*_key.pem", QDir::Readable | QDir::Files)) {
        const QString file = fi.absoluteFilePath();
        const QByteArray data = util::readFile(file);
        if (data.isEmpty()) {
            logWarn("could not read key: %1", file);
            continue;
        }
        impl_->loadedKeys << data;
    }
    foreach (const QFileInfo & fi, dir.entryInfoList(QStringList() << "*_crl.pem", QDir::Readable | QDir::Files)) {
        const QString file = fi.absoluteFilePath();
        const QByteArray data = util::readFile(file);
        if (data.isEmpty()) {
            logWarn("could not read revocation list: %1", file);
            continue;
        }
        impl_->loadedCrls << data;
    }
    return true;
}

bool TLSCredentials::activateLoaded(bool isTrustedCA)
{
    bool ok = true;
    foreach (const QByteArray & data, impl_->loadedCerts) {
        if (addCerts(data, isTrustedCA) == 0) {
            logWarn("could not handle certificate: %1", data);
            ok = false;
        }
    }
    impl_->loadedCerts.clear();
    foreach (const QByteArray & data, impl_->loadedKeys) {
        if (!addPrivateKey(data)) {
            logWarn("could not handle key: %1", data.left(40));
            ok = false;
        }
    }
    impl_->loadedKeys.clear();
    foreach (const QByteArray & data, impl_->loadedCrls) {
        if (addRevocationLists(data) == 0) {
            logWarn("could not handle revocation list: %1", data);
            ok = false;
        }
    }
    impl_->loadedCrls.clear();

    const QList<TLSCertInfo> infos = getAllCertInfos();
    logInfo("loaded %1 certifices:", infos.size());
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

QList<TLSCertInfo> TLSCredentials::getCertChainInfos() const
{
    QList<TLSCertInfo> rv;
    for (const Impl::CertsPrivKey & ck : impl_->chains) {
        for (const X509_Certificate & crt : ck.certs) {
            TLSCertInfo info = getInfo(crt);
            if (!info.isNull()) rv << info;
        }
    }
    return rv;
}

QList<TLSCertInfo> TLSCredentials::getAllCertInfos() const
{
    QList<TLSCertInfo> rv;
    for (const X509_Certificate & crt : impl_->allCerts) {
        TLSCertInfo info = getInfo(crt);
        if (!info.isNull()) rv << info;
    }
    return rv;
}

QByteArray TLSCredentials::getAllCertsPEM() const
{
    TRY {
        QByteArray rv = "";    // not null
        for (const X509_Certificate & cert : impl_->allCerts) {
            DER_Encoder enc;
            cert.encode_into(enc);
            rv += QByteArray::fromStdString(PEM_Code::encode(enc.get_contents(), "CERTIFICATE"));
            rv += '\n';
        }
        return rv;
    } CATCH
    return QByteArray();
}

Botan::Credentials_Manager & TLSCredentials::credentials_Manager()
{
    return *impl_;
}

}}    // namespace
