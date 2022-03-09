/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/crypt/tlsstream.h>

namespace cflib { namespace crypt {

class TLSCredentials;
class TLSSessions;

class TLSServer : public TLSStream
{
public:
	TLSServer(TLSSessions & sessions, TLSCredentials & credentials,
		bool highSecurity = false, bool requireRevocationInfo = false);
	~TLSServer();

	QByteArray initialSend() override { return QByteArray(); }
	bool received(const QByteArray & encrypted, QByteArray & plain, QByteArray & sendBack) override;
	bool send(const QByteArray & plain, QByteArray & encrypted) override;

private:
	class Impl;
	Impl * impl_;
};

}}	// namespace
