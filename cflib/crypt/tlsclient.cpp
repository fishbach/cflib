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

#include "tlsclient.h"

#include <cflib/crypt/impl/botanhelper.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/tlssessions.h>

#include <functional>

USE_LOG(LogCat::Crypt)

namespace cflib { namespace crypt {

class TLSClient::Impl
{
public:
	Impl(TLS::Session_Manager & session_manager, Credentials_Manager & creds, const QByteArray & hostname,
		RandomNumberGenerator & rng)
	:
		outgoingEncryptedPtr(&outgoingEncrypteedTmpBuf),
		incomingPlainPtr(0),
		isReady(false),
		hasError(false),
		policy(),
		client(
			std::bind(&Impl::socket_output_fn, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&Impl::data_cb, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&Impl::alert_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
			std::bind(&Impl::handshake_cb, this, std::placeholders::_1),
			session_manager, creds, policy,
			rng, hostname.isEmpty() ? std::string() : hostname.toStdString())
	{
	}

	void socket_output_fn(const byte buf[], size_t size)
	{
		outgoingEncryptedPtr->append((const char *)buf, size);
	}

	void data_cb(const byte buf[], size_t size)
	{
		incomingPlainPtr->append((const char *)buf, size);
	}

	void alert_cb(TLS::Alert alert, const byte[], size_t)
	{
		logInfo("TLS alert: %1", alert.type_string().c_str());
		hasError = true;
	}

	bool handshake_cb(const TLS::Session &)
	{
		isReady = true;
		if (!outgoingPlainTmpBuf.isEmpty()) {
			TRY {
				client.send((const byte *)outgoingPlainTmpBuf.constData(), outgoingPlainTmpBuf.size());
				outgoingPlainTmpBuf.clear();
				return true;
			} CATCH
			hasError = true;
		}
		return true;
	}

public:
	QByteArray outgoingEncrypteedTmpBuf;
	QByteArray outgoingPlainTmpBuf;
	QByteArray * outgoingEncryptedPtr;
	QByteArray * incomingPlainPtr;
	bool isReady;
	bool hasError;
	const TLS::Policy policy;
	TLS::Client client;
};

TLSClient::TLSClient(TLSSessions & sessions, TLSCredentials & credentials, const QByteArray & hostname) :
	impl_(0)
{
	TRY {
		impl_ = new Impl(sessions.session_Manager(), credentials.credentials_Manager(), hostname, sessions.rng());
	} CATCH
}

TLSClient::~TLSClient()
{
	delete impl_;
}

QByteArray TLSClient::initialEncryptedForServer()
{
	QByteArray rv = impl_->outgoingEncrypteedTmpBuf;
	impl_->outgoingEncrypteedTmpBuf.clear();
	return rv;
}

bool TLSClient::fromServer(const QByteArray & encrypted, QByteArray & plain, QByteArray & sendBack)
{
	if (impl_->hasError) return false;
	impl_->outgoingEncryptedPtr = &sendBack;
	impl_->incomingPlainPtr     = &plain;
	TRY {
		impl_->client.received_data((const byte *)encrypted.constData(), encrypted.size());
		return !impl_->hasError;
	} CATCH
	impl_->hasError = true;
	return false;
}

bool TLSClient::toServer(const QByteArray & plain, QByteArray & encrypted)
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
