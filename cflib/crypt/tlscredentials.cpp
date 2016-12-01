/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
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

class TLSCredentials::Impl : public Credentials_Manager
{
public:
	~Impl()
	{
		foreach (const CertsPrivKey & ck, chains) delete ck.privateKey;
	}

	virtual std::vector<Certificate_Store *> trusted_certificate_authorities(const std::string &, const std::string &)
	{
		std::vector<Certificate_Store *> rv(1);
		rv[0] = &trustedCAs;
		return rv;
	}

	virtual std::vector<X509_Certificate> cert_chain(const std::vector<std::string> & cert_key_types,
		const std::string & type, const std::string & context)
	{
		if (type != "tls-server") return std::vector<X509_Certificate>();
		foreach (const CertsPrivKey & ck, chains) {
			if (context != "" && !ck.certs[0].matches_dns_name(context)) continue;
			foreach (const std::string & kt, cert_key_types) {
				if (kt == ck.privateKey->algo_name()) return ck.certs;
			}
		}
		return std::vector<X509_Certificate>();
	}

	virtual Private_Key * private_key_for(const X509_Certificate & cert, const std::string &, const std::string &)
	{
		foreach (const CertsPrivKey & ck, chains) if (ck.certs[0] == cert) return ck.privateKey;
		return 0;
	}

public:
	struct CertsPrivKey {
		std::vector<X509_Certificate> certs;
		Private_Key * privateKey;

		CertsPrivKey() : privateKey(0) {}
	};
	QList<CertsPrivKey> chains;
	QList<X509_Certificate> allCerts;
	Certificate_Store_In_Memory trustedCAs;
	QList<QByteArray> loadedCerts;
	QList<QByteArray> loadedKeys;
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
		AutoSeeded_RNG rng;
		std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds, rng, password.toStdString()));

		// destroy data in parameters
		for (int i = 0 ; i < privateKey.size() ; ++i) *((char *)privateKey.constData()) = 0;
		for (int i = 0 ; i < password.size()   ; ++i) *((char *)password  .constData()) = 0;

		if (!pk) return false;

		// Does key exist?
		const std::vector<byte> pubKey = pk->x509_subject_public_key();
		foreach (const Impl::CertsPrivKey & ck, impl_->chains) {
			if (pubKey == ck.privateKey->x509_subject_public_key()) return false;
		}

		// search cert
		std::vector<X509_Certificate> certs;
		foreach (const X509_Certificate & crt, impl_->allCerts) {
			std::unique_ptr<Public_Key> certPubKey(crt.subject_public_key());
			if (pubKey == certPubKey->x509_subject_public_key()) {
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
		ck.privateKey = pk.release();
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
			logCritical("could not read certificate: %1", file);
			QTextStream(stderr) << "could not read certificate: " << file << endl;
			return false;
		}
		impl_->loadedCerts << data;
	}
	foreach (const QFileInfo & fi, dir.entryInfoList(QStringList() << "*_key.pem", QDir::Readable | QDir::Files)) {
		const QString file = fi.absoluteFilePath();
		const QByteArray data = util::readFile(file);
		if (data.isEmpty()) {
			logCritical("could not read key: %1", file);
			QTextStream(stderr) << "could not read key: " << file << endl;
			return false;
		}
		impl_->loadedKeys << data;
	}
	return true;
}

bool TLSCredentials::activateLoaded()
{
	foreach (const QByteArray & data, impl_->loadedCerts) {
		if (!addCerts(data)) {
			logCritical("could not handle certificate: %1", data);
			QTextStream(stderr) << "could not handle certificate: " << data << endl;
			return false;
		}
	}
	impl_->loadedCerts.clear();
	foreach (const QByteArray & data, impl_->loadedKeys) {
		if (!addPrivateKey(data)) {
			logCritical("could not handle key: %1", data);
			QTextStream(stderr) << "could not handle key: " << data << endl;
			return false;
		}
	}
	impl_->loadedKeys.clear();
	return true;
}

QList<TLSCertInfo> TLSCredentials::getCertInfos() const
{
	QList<TLSCertInfo> rv;
	std::vector<Certificate_Store *> trusted = impl_->trusted_certificate_authorities("", "");
	foreach (const Impl::CertsPrivKey & ck, impl_->chains) {
		foreach (const X509_Certificate & crt, ck.certs) {
			TRY {
				TLSCertInfo info;
				info.subjectName = fromStdVector(crt.subject_dn().get_attribute("X520.CommonName"));
				info.issuerName  = fromStdVector(crt.issuer_dn ().get_attribute("X520.CommonName"));
				info.isCA        = crt.is_CA_cert();
				foreach (Certificate_Store * cs, trusted) {
					if (cs->find_cert(crt.subject_dn(), crt.subject_key_id())) {
						info.isTrusted = true;
						break;
					}
				}
				rv << info;
			} CATCH
		}
	}
	return rv;
}

Botan::Credentials_Manager & TLSCredentials::credentials_Manager()
{
	return *impl_;
}

}}	// namespace
