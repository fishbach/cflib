/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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
	Impl() : privateKey(0) {}

	void sortCerts()
	{
		for (uint i = 0 ; i < certs.size() ; ) {
			bool again = false;
			for (uint j = i + 1 ; j < certs.size() ; ++j) {
				if (certs[j].authority_key_id() == certs[i].subject_key_id()) {
					// loop!?!
					if (certs[i].authority_key_id() == certs[j].subject_key_id()) return;
					std::swap(certs[i], certs[j]);
					again = true;
					break;
				}
			}
			if (!again) ++i;
		}
	}

	std::vector<Certificate_Store *> trusted_certificate_authorities(const std::string &, const std::string &)
	{
		std::vector<Certificate_Store *> rv(1);
		rv[0] = &trustedCAs;
		return rv;
	}

	virtual std::vector<X509_Certificate> cert_chain(const std::vector<std::string> & cert_key_types,
		const std::string & type, const std::string & context)
	{
		if (!privateKey || certs.size() == 0 ||
			std::find(cert_key_types.begin(), cert_key_types.end(), privateKey->algo_name()) == cert_key_types.end() ||
			type != "tls-server" ||
			(context != "" && !certs[0].matches_dns_name(context)))
		{
			return std::vector<X509_Certificate>();
		}
		return certs;
	}

	virtual Private_Key * private_key_for(const X509_Certificate &, const std::string &, const std::string &)
	{
		return privateKey;
	}

public:
	std::vector<X509_Certificate> certs;
	Certificate_Store_In_Memory trustedCAs;
	Private_Key * privateKey;
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

			bool exists = false;
			foreach (const X509_Certificate & c, impl_->certs) if (c == crt) { exists = true; break; }
			if (exists) continue;

			impl_->certs.push_back(crt);
			if (isTrustedCA) impl_->trustedCAs.add_certificate(crt);
			++rv;
		}
	} catch (...) {}
	impl_->sortCerts();
	return rv;
}

bool TLSCredentials::setPrivateKey(const QByteArray & privateKey)
{
	if (impl_->privateKey) {
		delete impl_->privateKey;
		impl_->privateKey = 0;
	}

	if (impl_->certs.size() == 0) return false;

	TRY {
		DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
		AutoSeeded_RNG rng;
		impl_->privateKey = PKCS8::load_key(ds, rng);

		// check if first cert matches private key
		std::unique_ptr<Public_Key> certPubKey(impl_->certs[0].subject_public_key());
		if (impl_->privateKey->x509_subject_public_key() != certPubKey->x509_subject_public_key()) {
			delete impl_->privateKey;
			impl_->privateKey = 0;
			return false;
		}

		return true;
	} CATCH
	return false;
}

QList<TLSCertInfo> TLSCredentials::getCertInfos() const
{
	QList<TLSCertInfo> rv;
	std::vector<Certificate_Store *> trusted = impl_->trusted_certificate_authorities("", "");
	foreach (const X509_Certificate & crt, impl_->certs) {
		TLSCertInfo info;
		TRY {
			info.subjectName = fromStdVector(crt.subject_dn().get_attribute("X520.CommonName"));
			info.issuerName  = fromStdVector(crt.issuer_dn ().get_attribute("X520.CommonName"));
			info.isCA        = crt.is_CA_cert();
			for (std::vector<Certificate_Store *>::const_iterator it = trusted.begin() ; it != trusted.end() ; ++it) {
				if ((*it)->find_cert(crt.subject_dn(), crt.subject_key_id())) {
					info.isTrusted = true;
					break;
				}
			}
		} CATCH
		rv << info;
	}
	return rv;
}

Botan::Credentials_Manager & TLSCredentials::credentials_Manager()
{
	return *impl_;
}

}}	// namespace
