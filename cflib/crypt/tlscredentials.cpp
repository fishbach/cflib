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
	QList<X509_Certificate> certs;
};

TLSCredentials::TLSCredentials() :
	impl_(new Impl)
{
}

TLSCredentials::~TLSCredentials()
{
	delete impl_;
}

uint TLSCredentials::addCert(const QByteArray & pem)
{
	uint rv = 0;
	TRY {
		DataSource_Memory ds((const byte *)pem.constData(), pem.size());
		forever {
			impl_->certs << X509_Certificate(ds);
			QTextStream(stdout) << QString::fromStdString(impl_->certs.last().to_string()) << endl << "===" << endl;
			++rv;
		}
	} CATCH
	return rv;
}

bool TLSCredentials::addPrivateKey(const QByteArray & /*privateKey*/)
{
	return false;
}

}}	// namespace
