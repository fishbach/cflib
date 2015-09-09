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
#include <cflib/net/tcpconn.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

// Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits
// Sec-WebSocket-Extensions: permessage-deflate
// Sec-WebSocket-Extensions: x-webkit-deflate-frame

namespace cflib { namespace net {

class WebSocketService::WSConnHandler : public util::ThreadVerify, public TCPConn
{
public:
	WSConnHandler(WebSocketService * service, TCPConnData * connData, uint connId, uint connectionTimeoutSec,
		bool deflate)
	:
		ThreadVerify(service),
		TCPConn(connData, 0x10000, connectionTimeoutSec > 0),
		service_(*service),
		connId_(connId),
		connectionSendInterval_(connectionTimeoutSec / 2),
		connectionDataTimeout_(connectionTimeoutSec * 3 / 2),
		isBinary_(false),
		ping_("\x89\x00", 2)
	{
		logFunctionTrace
		setNoDelay(true);
		startReadWatcher();
		if (connectionTimeoutSec > 0) {
			lastRead_  = QDateTime::currentDateTime();
			lastWrite_ = QDateTime::currentDateTime();
		}
		if (deflate) logDebug("using deflate on connection: %1", connId);
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

		// write first start byte
		frame += isBinary ? 0x82 : 0x81;

		// write length
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

	void checkTimeout(const QDateTime & now)
	{
		uint last = qMax(lastRead_.secsTo(now), lastWrite_.secsTo(now));
		if (last < connectionSendInterval_) return;
		if (last > connectionDataTimeout_) {
			logInfo("timeout on connection %1", connId_);
			close(HardClosed, true);
		} else {
			write(ping_);
		}
	}

protected:
	virtual void newBytesAvailable()
	{
		if (!verifyThreadCall(&WSConnHandler::newBytesAvailable)) return;

		buf_ += read();
		if (connectionDataTimeout_ > 0) lastRead_ = QDateTime::currentDateTime();
		handleData();
		startReadWatcher();
	}

	virtual void closed(CloseType type)
	{
		if (!verifyThreadCall(&WSConnHandler::closed, type)) return;

		if ((type & ReadClosed) && (type & WriteClosed)) {
			service_.connections_.remove(connId_);
			util::deleteNext(this);
		}
		service_.closed(connId_, type);
	}

	virtual void someBytesWritten(quint64 count)
	{
		if (!verifyThreadCall(&WSConnHandler::someBytesWritten, count)) return;
		lastWrite_ = QDateTime::currentDateTime();
	}

private:
	void handleData()
	{
		while (buf_.size() >= 2) {
			quint8 * data = (quint8 *)buf_.constData();
			uint dLen = buf_.size();
			bool fin = data[0] & 0x80;
			quint8 opcode = data[0] & 0xF;
			bool mask = data[1] & 0x80;

			// clients must send masked data
			if (!mask) {
				logWarn("no mask in frame: %1", buf_);
				close(HardClosed, true);
				return;
			}

			// read len
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

			// Enough data available?
			if (dLen < len + 4) return;

			// apply mask
			const quint8 * maskKey = data;
			data += 4;
			dLen -= 4;
			for (uint i = 0 ; i < len ; ++i) data[i] ^= maskKey[i % 4];

			// handle message types
			if (opcode == 0x0) {	// continuation frame
				fragmentBuf_.append((const char *)data, len);
				if (fin) {
					service_.newMsg(connId_, fragmentBuf_, isBinary_);
					fragmentBuf_.clear();
				}
			} else if (opcode == 0x1 || opcode == 0x2) {	// test / binary frame
				if (!fin) {
					isBinary_ = opcode == 2;
					fragmentBuf_.append((const char *)data, len);
				} else {
					service_.newMsg(connId_, QByteArray((const char *)data, len), opcode == 2);
				}
			} else if (opcode == 0x8) {	// connection close
				logDebug("received close frame");
				close(ReadWriteClosed, true);
			} else if (opcode == 0x9) {	// ping
				// send pong
				quint8 * orig = (quint8 *)buf_.constData();
				orig[0] = (orig[0] & 0xF0) | 0xA;
				orig[1] &= 0x7F;
				QByteArray pong((const char *)orig, buf_.size() - dLen - 4);
				pong.append((const char *)data, len);
				write(pong);
			} else if (opcode == 0xA) {
				// pong
			} else  {
				logWarn("unknown opcode %1 in frame (%2)", opcode, buf_.toHex());
			}

			buf_.remove(0, buf_.size() - dLen + len);
		}
	}

private:
	WebSocketService & service_;
	const uint connId_;
	const uint connectionSendInterval_;
	const uint connectionDataTimeout_;
	QByteArray buf_;
	QByteArray fragmentBuf_;
	bool isBinary_;
	QDateTime lastRead_;
	QDateTime lastWrite_;
	const QByteArray ping_;
};

// ============================================================================

WebSocketService::WebSocketService(const QString & path, uint connectionTimeoutSec) :
	ThreadVerify("WebSocketService", LoopType::Worker),
	path_(path),
	connectionTimeoutSec_(connectionTimeoutSec),
	lastConnId_(0),
	timer_(connectionTimeoutSec > 0 ? new util::EVTimer(this, &WebSocketService::checkTimeout) : 0)
{
	setThreadPrio(QThread::HighPriority);
	if (timer_) startTimer();
}

WebSocketService::~WebSocketService()
{
	delete timer_;
}

void WebSocketService::send(uint connId, const QByteArray & data, bool isBinary)
{
	WSConnHandler * wsHdl = connections_.value(connId);
	if (wsHdl) wsHdl->send(data, isBinary);
}

void WebSocketService::close(uint connId, TCPConn::CloseType type)
{
	WSConnHandler * wsHdl = connections_.value(connId);
	if (wsHdl) wsHdl->close(type, true);
}

QByteArray WebSocketService::getRemoteIP(uint connId)
{
	WSConnHandler * wsHdl = connections_.value(connId);
	if (wsHdl) return wsHdl->peerIP();
	return QByteArray();
}

void WebSocketService::newConnection(uint)
{
}

void WebSocketService::closed(uint, TCPConn::CloseType)
{
}

void WebSocketService::handleRequest(const Request & request)
{
	if (request.getUri() != path_ || !request.isGET()) return;
	addConnection(request);
}

void WebSocketService::addConnection(const Request & request)
{
	if (!verifyThreadCall(&WebSocketService::addConnection, request)) return;

	logFunctionTrace

	// check WS headers
	const Request::KeyVal headers = request.getHeaderFields();
	const QByteArray wsKey = headers["sec-websocket-key"];
	if (headers["upgrade"].toLower() != "websocket" || wsKey.isEmpty()) {
		request.sendNotFound();
		return;
	}
	const bool deflate = headers["sec-websocket-extensions"].toLower().indexOf("permessage-deflate") != -1;

	// detach from socket
	TCPConnData * connData = request.detach();
	if (!connData) {
		logWarn("could not detach from socket");
		return;
	}
	const uint connId = ++lastConnId_;
	WSConnHandler * wsHdl = new WSConnHandler(this, connData, connId, connectionTimeoutSec_, deflate);
	connections_[connId] = wsHdl;

	// write WS header
	QByteArray header =
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: ";
	header += cflib::crypt::sha1(wsKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toBase64();
	header += "\r\n";
	if (deflate) header += "Sec-WebSocket-Extensions: permessage-deflate\r\n";
	header += "\r\n";
	wsHdl->write(header);

	newConnection(connId);
}

void WebSocketService::startTimer()
{
	if (!verifyThreadCall(&WebSocketService::startTimer)) return;
	timer_->start(connectionTimeoutSec_ / 4.0);
}

void WebSocketService::checkTimeout()
{
	const QDateTime now = QDateTime::currentDateTime();
	QHashIterator<uint, WSConnHandler *> it(connections_);
	while (it.hasNext()) it.next().value()->checkTimeout(now);
}

}}	// namespace
