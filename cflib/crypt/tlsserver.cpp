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

#include "tlsserver.h"

#include <cflib/crypt/impl/botanhelper.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/tlssessions.h>

#include <functional>

USE_LOG(LogCat::Crypt)

namespace cflib { namespace crypt {

class TLSServer::Impl : public TLS::Server
{
public:
	Impl(TLS::Session_Manager & session_manager, Credentials_Manager & creds, RandomNumberGenerator & rng) :
		Server(
			std::bind(&Impl::socket_output_fn, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&Impl::data_cb, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&Impl::alert_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
			std::bind(&Impl::handshake_cb, this, std::placeholders::_1),
			session_manager, creds, TLS::Policy(),
			rng),
		isReady(false),
		hasError(false)
	{
	}

	void socket_output_fn(const byte buf[], size_t size)
	{
		toClient.append((const char *)buf, size);
	}

	void data_cb(const byte buf[], size_t size)
	{
		incomingPlain.append((const char *)buf, size);
	}

	void alert_cb(TLS::Alert alert, const byte[], size_t)
	{
		logInfo("TLS alert: %1", alert.type_string().c_str());
		hasError = true;
	}

	bool handshake_cb(const TLS::Session &)
	{
		isReady = true;
		if (!outgoingPlain.isEmpty()) {
			TRY {
				send((const byte *)outgoingPlain.constData(), outgoingPlain.size());
				outgoingPlain.clear();
				return true;
			} CATCH
			hasError = true;
		}
		return true;
	}

public:
	QByteArray toClient;
	QByteArray incomingPlain;
	QByteArray outgoingPlain;
	bool isReady;
	bool hasError;
};

TLSServer::TLSServer(TLSSessions & sessions, TLSCredentials & credentials) :
	impl_(0)
{
	TRY {
		impl_ = new Impl(sessions.session_Manager(), credentials.credentials_Manager(), sessions.rng());
	} CATCH
}

TLSServer::~TLSServer()
{
	delete impl_;
}

QByteArray TLSServer::initialDataForClient()
{
	QByteArray rv = impl_->toClient;
	impl_->toClient.clear();
	return rv;
}

bool TLSServer::fromClient(const QByteArray & encoded, QByteArray & plain, QByteArray & sendBack)
{
	if (impl_->hasError) return false;
	TRY {
		impl_->received_data((const byte *)encoded.constData(), encoded.size());
		if (!impl_->incomingPlain.isEmpty()) {
			plain.append(impl_->incomingPlain);
			impl_->incomingPlain.clear();
		}
		if (!impl_->toClient.isEmpty()) {
			sendBack.append(impl_->toClient);
			impl_->toClient.clear();
		}
		return !impl_->hasError;
	} CATCH
	impl_->hasError = true;
	return false;
}

bool TLSServer::toClient(const QByteArray & plain, QByteArray & encoded)
{
	if (impl_->hasError) return false;
	TRY {
		if (impl_->isReady) impl_->send((const byte *)plain.constData(), plain.size());
		else                impl_->outgoingPlain.append(plain);
		if (!impl_->toClient.isEmpty()) {
			encoded.append(impl_->toClient);
			impl_->toClient.clear();
		}
		return !impl_->hasError;
	} CATCH
	impl_->hasError = true;
	return false;
}

}}	// namespace
