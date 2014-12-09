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

#include "websocketservice.h"

#include <cflib/crypt/util.h>
#include <cflib/http/apiserver.h>
#include <cflib/http/request.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace http {

namespace {

class WSRequestHandler : public util::ThreadVerify
{
public:
	WSRequestHandler(WebSocketService * service, const Request & request, const QString & name) :
		ThreadVerify(service),
		service_(*service),
		request_(request),
		name_(name)
	{
		logFunctionTrace

		const Request::KeyVal headers = request_.getHeaderFields();
		QByteArray wsKey = headers["sec-webSocket-key"];
		if (headers["upgrade"          ] != "websocket" ||
			headers["connection"       ] != "Upgrade"   ||
			wsKey.isEmpty())
		{
			request.sendNotFound();
			deleteNext();
			return;
		}

		wsKey = cflib::crypt::sha1(wsKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toBase64();

	}

	~WSRequestHandler()
	{
		logFunctionTrace
	}

private:
	WebSocketService & service_;
	const Request request_;
	const QString & name_;
};

}

WebSocketService::WebSocketService(const QString & basePath) :
	ThreadVerify("WebSocketService", util::ThreadVerify::Worker, 1),
	basePath_(basePath)
{
}

WebSocketService::~WebSocketService()
{
	stopVerifyThread();
}

void WebSocketService::handleRequest(const Request & request)
{
	QString path = request.getUrl().path();
	if (!path.startsWith(basePath_) || !request.isGET()) return;

	new WSRequestHandler(this, request, path.mid(basePath_.length()));
}

}}	// namespace
