/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include "tlsclient.h"

#include <cflib/crypt/impl/botanhelper.h>
#include <cflib/crypt/impl/tls12policy.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/tlssessions.h>

USE_LOG(LogCat::Crypt)

namespace cflib { namespace crypt {

class TLSClient::Impl : public TLS::Callbacks
{
public:
	Impl(TLS::Session_Manager & session_manager, Credentials_Manager & creds, const QByteArray & hostname,
		bool highSecurity, bool requireRevocationInfo)
	:
		outgoingEncryptedPtr(&outgoingEncrypteedTmpBuf),
		incomingPlainPtr(0),
		isReady(false),
		hasError(false),
		policy(highSecurity ? (TLS::Policy *)new TLS::Strict_Policy : (TLS::Policy *)new TLS::TLS12Policy(requireRevocationInfo)),
		client(*this, session_manager, creds, *policy, rng, hostname.isEmpty() ? std::string() : hostname.toStdString())
	{
	}

	void tls_emit_data(const byte data[], size_t size) override
	{
		outgoingEncryptedPtr->append((const char *)data, (int)size);
	}

	void tls_record_received(u64bit, const byte data[], size_t size) override
	{
		incomingPlainPtr->append((const char *)data, (int)size);
	}

	void tls_alert(TLS::Alert alert) override
	{
		if (alert.type() != TLS::Alert::CLOSE_NOTIFY) {
			logWarn("TLS alert: %1", alert.type_string().c_str());
		} else {
			logTrace("TLS alert: %1", alert.type_string().c_str());
		}
		hasError = true;
	}

	bool tls_session_established(const TLS::Session &) override
	{
		isReady = true;
		return true;
	}

	std::chrono::milliseconds tls_verify_cert_chain_ocsp_timeout() const override
	{
		return std::chrono::milliseconds(1000);
	}

public:
	QByteArray outgoingEncrypteedTmpBuf;
	QByteArray outgoingPlainTmpBuf;
	QByteArray * outgoingEncryptedPtr;
	QByteArray * incomingPlainPtr;
	bool isReady;
	bool hasError;
	std::unique_ptr<TLS::Policy> policy;
	AutoSeeded_RNG rng;
	TLS::Client client;
};

TLSClient::TLSClient(TLSSessions & sessions, TLSCredentials & credentials, const QByteArray & hostname,
	bool highSecurity, bool requireRevocationInfo)
:
	impl_(0)
{
	TRY {
		impl_ = new Impl(sessions.session_Manager(), credentials.credentials_Manager(), hostname, highSecurity, requireRevocationInfo);
	} CATCH
}

TLSClient::~TLSClient()
{
	delete impl_;
}

QByteArray TLSClient::initialSend()
{
	QByteArray rv = impl_->outgoingEncrypteedTmpBuf;
	impl_->outgoingEncrypteedTmpBuf.clear();
	return rv;
}

bool TLSClient::received(const QByteArray & encrypted, QByteArray & plain, QByteArray & sendBack)
{
	if (impl_->hasError) return false;
	impl_->outgoingEncryptedPtr = &sendBack;
	impl_->incomingPlainPtr     = &plain;
	TRY {
		impl_->client.received_data((const byte *)encrypted.constData(), encrypted.size());
		QByteArray & tmpBuf = impl_->outgoingPlainTmpBuf;
		if (!tmpBuf.isEmpty() && impl_->isReady && !impl_->hasError) {
			impl_->client.send((const byte *)tmpBuf.constData(), tmpBuf.size());
			tmpBuf.clear();
		}
		return !impl_->hasError;
	} CATCH
	impl_->hasError = true;
	return false;
}

bool TLSClient::send(const QByteArray & plain, QByteArray & encrypted)
{
	if (impl_->hasError) return false;
	impl_->outgoingEncryptedPtr = &encrypted;
	TRY {
		if (impl_->isReady) impl_->client.send((const byte *)plain.constData(), plain.size());
		else                impl_->outgoingPlainTmpBuf.append(plain);
		return !impl_->hasError;
	} CATCH
	impl_->hasError = true;
	return false;
}

}}	// namespace
