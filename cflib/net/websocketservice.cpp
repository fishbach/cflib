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
#include <cflib/net/apiserver.h>
#include <cflib/net/request.h>
#include <cflib/net/tcpserver.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

class WebSocketService::WSConnHandler : public TCPConn
{
public:
	WSConnHandler(WebSocketService * service, const TCPConnInitializer * connInit) :
		TCPConn(connInit),
		service_(*service),
		clientId_(0),
		isBinary_(false),
		isFirstMsg_(true),
		abort_(false)
	{
		logFunctionTrace
		setNoDelay(true);
		startWatcher();
	}

	~WSConnHandler()
	{
		logTrace("~WSConnHandler()");
	}

	void send(const QByteArray & data, bool isBinary)
	{
		const uint len = data.size();
		QByteArray frame;
		frame.reserve(len + 10);
		frame += isBinary ? 0x82 : 0x81;
		if (len < 126) {
			frame += len;
		} else if (len < 0x10000) {
			frame += 126;
			frame += len >> 8;
			frame += len & 0xFF;
		} else {
			frame += 127;
			frame += (char)0;	// (len >> 56) & 0xFF;
			frame += (char)0;	// (len >> 48) & 0xFF;
			frame += (char)0;	// (len >> 40) & 0xFF;
			frame += (char)0;	// (len >> 32) & 0xFF;
			frame += (len >> 24) & 0xFF;
			frame += (len >> 16) & 0xFF;
			frame += (len >>  8) & 0xFF;
			frame += len & 0xFF;
		}
		frame += data;
		write(frame);
	}

	void close()
	{
		abort_ = true;
		closeNicely();
	}

protected:
	virtual void newBytesAvailable()
	{
		buf_ += read();
		handleData();
		startWatcher();
	}

	virtual void closed()
	{
		logFunctionTrace
		service_.all_.remove(this);
		if (clientId_ == 0) {
			if (!abort_) service_.closed(0);
		} else {
			service_.clients_.remove(clientId_, this);
			if (!service_.clients_.contains(clientId_) && !abort_) {
				service_.apiServer_.blockExpiration(clientId_, false);
				service_.closed(clientId_);
			}
		}
		util::deleteNext(this);
	}

private:
	inline bool idMsg(const QByteArray & data)
	{
		if (!isFirstMsg_) return false;
		isFirstMsg_ = false;

		if (!data.startsWith("CFLIB_clientId#")) {
			abort_ = !service_.newClient(0);
			if (abort_) {
				abortConnection();
				return true;
			}
			service_.all_ << this;
			return false;
		}

		service_.apiServer_.getClientId(data.mid(15), clientId_);
		if (clientId_ == 0) {
			logInfo("unknown client id: %1", data);
			abort_ = true;
			abortConnection();
			return true;
		}

		if (!service_.clients_.contains(clientId_)) {
			abort_ = !service_.newClient(clientId_);
			if (abort_) {
				abortConnection();
				return true;
			}
			service_.apiServer_.blockExpiration(clientId_, true);
		}

		service_.all_ << this;
		service_.clients_.insert(clientId_, this);
		return true;
	}

	void handleData()
	{
		while (buf_.size() >= 2 && !abort_) {
			quint8 * data = (quint8 *)buf_.constData();
			uint dLen = buf_.size();
			bool fin = data[0] & 0x80;
			quint8 opcode = data[0] & 0xF;
			bool mask = data[1] & 0x80;

			// clients must send masked data
			if (!mask) {
				logWarn("no mask in frame: %1", buf_);
				abortConnection();
				return;
			}

			quint64 len = data[1] & 0x7F;
			if (len < 126) {
				data += 2;
				dLen -= 2;
			} else if (len == 126) {
				if (dLen < 4) return;
				len = (quint64)data[2] << 8 | (quint64)data[3];
				data += 4;
				dLen -= 4;
			} else {
				if (dLen < 10) return;
				len =
					(quint64)data[2] << 56 | (quint64)data[3] << 48 | (quint64)data[4] << 40 | (quint64)data[5] << 32 |
					(quint64)data[6] << 24 | (quint64)data[7] << 16 | (quint64)data[8] <<  8 | (quint64)data[9];
				data += 10;
				dLen -= 10;
			}

			// apply mask
			if (dLen < len + 4) return;
			const quint8 * maskKey = data;
			data += 4;
			dLen -= 4;
			for (uint i = 0 ; i < len ; ++i) data[i] ^= maskKey[i % 4];

			if (opcode == 0x0) {
				fragmentBuf_.append((const char *)data, len);
				if (fin) {
					if (!idMsg(fragmentBuf_)) service_.newMsg(clientId_, fragmentBuf_, isBinary_);
					fragmentBuf_.clear();
				}
			} else if (opcode == 0x1 || opcode == 0x2) {
				if (!fin) {
					isBinary_ = opcode == 2;
					fragmentBuf_.append((const char *)data, len);
				} else {
					QByteArray payload((const char *)data, len);
					if (!idMsg(payload)) service_.newMsg(clientId_, payload, opcode == 2);
				}
			} else if (opcode == 0x8) {	// close
				logDebug("received close frame");
				closeNicely();
			} else if (opcode == 0x9) {	// ping
				// send pong
				quint8 * orig = (quint8 *)buf_.constData();
				orig[0] = (orig[0] & 0xF) | 0xA;
				orig[1] &= 0x7F;
				QByteArray pong((const char *)orig, buf_.size() - dLen - 4);
				pong.append((const char *)data, len);
				write(pong);
			} else if (opcode == 0xA) {	// pong
				logDebug("pong received");
			} else  {
				logWarn("unknown opcode %1 in frame (%2)", opcode, buf_.toHex());
			}

			buf_.remove(0, buf_.size() - dLen + len);
		}
	}

private:
	WebSocketService & service_;
	const QString name_;
	QByteArray buf_;
	uint clientId_;
	bool isBinary_;
	QByteArray fragmentBuf_;
	bool isFirstMsg_;
	bool abort_;
};

// ============================================================================

WebSocketService::WebSocketService(ApiServer & apiServer, const QString & path) :
	apiServer_(apiServer),
	path_(path)
{
}

void WebSocketService::send(uint clientId, const QByteArray & data, bool isBinary)
{
	foreach (WSConnHandler * wsHdl, clients_.values(clientId)) wsHdl->send(data, isBinary);
}

void WebSocketService::sendAll(const QByteArray & data, bool isBinary)
{
	foreach (WSConnHandler * wsHdl, all_) wsHdl->send(data, isBinary);
}

void WebSocketService::close(uint clientId)
{
	foreach (WSConnHandler * wsHdl, clients_.values(clientId)) wsHdl->close();
}

bool WebSocketService::isConnected(uint clientId) const
{
	return clients_.contains(clientId);
}

bool WebSocketService::newClient(uint)
{
	return true;
}

void WebSocketService::closed(uint)
{
}

void WebSocketService::handleRequest(const Request & request)
{
	logFunctionTrace

	if (request.getUri() != path_ || !request.isGET()) return;

	const Request::KeyVal headers = request.getHeaderFields();
	const QByteArray wsKey = headers["sec-websocket-key"];
	if (headers["upgrade"] != "websocket" || wsKey.isEmpty()) {
		request.sendNotFound();
		return;
	}

	const TCPConnInitializer * connInit = request.detachFromSocket();
	if (!connInit) {
		logWarn("could not detach from socket");
		return;
	}
	WSConnHandler * wsHdl = new WSConnHandler(this, connInit);

	QByteArray header =
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: ";
	header += cflib::crypt::sha1(wsKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toBase64();
	header += "\r\n\r\n";
	wsHdl->write(header);
}

}}	// namespace
