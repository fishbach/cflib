/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "tlsserver.h"

#include <cflib/crypt/impl/botanhelper.h>
#include <cflib/crypt/impl/tls12policy.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/tlssessions.h>

USE_LOG(LogCat::Crypt)

namespace cflib { namespace crypt {

class TLSServer::Impl : public TLS::Callbacks
{
public:
	Impl(TLS::Session_Manager & session_manager, Credentials_Manager & creds, bool highSecurity, bool requireRevocationInfo) :
		outgoingEncryptedPtr(0),
		incomingPlainPtr(0),
		isReady(false),
		hasError(false),
		policy(highSecurity ? (TLS::Policy *)new TLS::Strict_Policy : (TLS::Policy *)new TLS::TLS12Policy(requireRevocationInfo)),
		server(*this, session_manager, creds, *policy, rng)
	{
	}

	void tls_emit_data(const byte data[], size_t size)
	{
		outgoingEncryptedPtr->append((const char *)data, (int)size);
	}

	void tls_record_received(u64bit, const byte data[], size_t size)
	{
		incomingPlainPtr->append((const char *)data, (int)size);
	}

	void tls_alert(TLS::Alert alert)
	{
		logInfo("TLS alert: %1", alert.type_string().c_str());
		hasError = true;
	}

	bool tls_session_established(const TLS::Session &)
	{
		isReady = true;
		return true;
	}

public:
	QByteArray outgoingPlainTmpBuf;
	QByteArray * outgoingEncryptedPtr;
	QByteArray * incomingPlainPtr;
	bool isReady;
	bool hasError;
	std::unique_ptr<TLS::Policy> policy;
	AutoSeeded_RNG rng;
	TLS::Server server;
};

TLSServer::TLSServer(TLSSessions & sessions, TLSCredentials & credentials, bool highSecurity, bool requireRevocationInfo) :
	impl_(0)
{
	TRY {
		impl_ = new Impl(sessions.session_Manager(), credentials.credentials_Manager(), highSecurity, requireRevocationInfo);
	} CATCH
}

TLSServer::~TLSServer()
{
	delete impl_;
}

bool TLSServer::received(const QByteArray & encrypted, QByteArray & plain, QByteArray & sendBack)
{
	if (impl_->hasError) return false;
	impl_->outgoingEncryptedPtr = &sendBack;
	impl_->incomingPlainPtr     = &plain;
	TRY {
		impl_->server.received_data((const byte *)encrypted.constData(), encrypted.size());
		QByteArray & tmpBuf = impl_->outgoingPlainTmpBuf;
		if (!tmpBuf.isEmpty() && impl_->isReady && !impl_->hasError) {
			impl_->server.send((const byte *)tmpBuf.constData(), tmpBuf.size());
			tmpBuf.clear();
		}
		return !impl_->hasError;
	} CATCH
	impl_->hasError = true;
	return false;
}

bool TLSServer::send(const QByteArray & plain, QByteArray & encrypted)
{
	if (impl_->hasError) return false;
	impl_->outgoingEncryptedPtr = &encrypted;
	TRY {
		if (impl_->isReady) impl_->server.send((const byte *)plain.constData(), plain.size());
		else                impl_->outgoingPlainTmpBuf.append(plain);
		return !impl_->hasError;
	} CATCH
	impl_->hasError = true;
	return false;
}

}}	// namespace
