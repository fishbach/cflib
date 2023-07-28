/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

	void tls_verify_cert_chain(
		const std::vector<X509_Certificate> & cert_chain,
		const std::vector<std::shared_ptr<const OCSP::Response>> & ocsp_responses,
		const std::vector<Certificate_Store *> & trusted_roots,
		Usage_Type usage,
		const std::string & hostname,
		const TLS::Policy & policy) override
	{
		try {
			Callbacks::tls_verify_cert_chain(cert_chain, ocsp_responses, trusted_roots, usage, hostname, policy);
		} catch(Exception & e) {
			// handle self signed certificates
			if (usage != Usage_Type::TLS_SERVER_AUTH || cert_chain.empty()) throw e;

			const X509_Certificate & crt = cert_chain[0];
			if (!crt.is_self_signed() || !crt.matches_dns_name(hostname)) throw e;

			for (Certificate_Store * cs : trusted_roots) {
				std::shared_ptr<const X509_Certificate> trusted = cs->find_cert(crt.subject_dn(), crt.subject_key_id());
				if (trusted && trusted->subject_public_key_bits() == crt.subject_public_key_bits()) return;
			}

			throw e;
		}
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
