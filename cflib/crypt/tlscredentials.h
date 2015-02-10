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

#pragma once

#include <QtCore>

namespace Botan { class Credentials_Manager; }

namespace cflib { namespace crypt {

struct TLSCertInfo
{
	QByteArray subjectName;
	QByteArray issuerName;
	bool isCA;
	bool isTrusted;

	TLSCertInfo() : isCA(false), isTrusted(false) {}
};

class TLSCredentials
{
	Q_DISABLE_COPY(TLSCredentials)
public:
	TLSCredentials();
	~TLSCredentials();

	uint addCerts(const QByteArray & certs, bool isTrustedCA = false);
	QList<TLSCertInfo> getCertInfos() const;

	// fitting certificate chain must exist
	bool addPrivateKey(const QByteArray & privateKey);

private:
	class Impl;
	Impl * impl_;

	friend class TLSClient;
	friend class TLSServer;
	Botan::Credentials_Manager & credentials_Manager();
};

}}	// namespace