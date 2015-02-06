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

class TLSCredentials::Impl : public Credentials_Manager
{
public:
	Impl() : privateKey(0) {}

public:
	QList<X509_Certificate> certs;
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

uint TLSCredentials::addCert(const QByteArray & cert)
{
	uint rv = 0;
	try {
		DataSource_Memory ds((const byte *)cert.constData(), cert.size());
		forever {
			impl_->certs << X509_Certificate(ds);
			QTextStream(stdout) << QString::fromStdString(impl_->certs.last().to_string()) << endl << "===" << endl;
			++rv;
		}
	} catch (...) {}
	return rv;
}

bool TLSCredentials::setPrivateKey(const QByteArray & privateKey)
{
	delete impl_->privateKey;
	impl_->privateKey = 0;
	try {
		DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
		AutoSeeded_RNG rng;
		impl_->privateKey = PKCS8::load_key(ds, rng);
		return true;
	} catch (...) {}
	return false;
}

}}	// namespace
