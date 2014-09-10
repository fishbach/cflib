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

#include "requestparser.h"

#include <cflib/http/request.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace http { namespace impl {

namespace {

QAtomicInt connCount;

}

RequestParser::RequestParser(qintptr sock, const QList<RequestHandler *> & handlers, util::ThreadVerify * tv) :
	QObject(tv->threadObject()),
	util::ThreadVerify(tv),
	handlers_(handlers),
	id_(connCount.fetchAndAddRelaxed(1) + 1),
	contentLength_(-1),
	requestCount_(0), nextReplyId_(1),
	attachedRequests_(1),
	socketClosed_(false)
{
	sock_.setSocketDescriptor(sock);
	connect(&sock_, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
			this, SLOT(stateChanged(QAbstractSocket::SocketState)));
	connect(&sock_, SIGNAL(readyRead()), this, SLOT(readyRead()));

	logCustom(LogCat::Network | LogCat::Debug)("new connection %1", id_);

	readyRead();
}

RequestParser::~RequestParser()
{
	logTrace("deleted RequestParser of connection %1", id_);
}

void RequestParser::sendReply(int id, const QByteArray & reply)
{
	if (!verifyThreadCall(&RequestParser::sendReply, id, reply)) return;

	if (id == nextReplyId_) {
		writeReply(reply);

		QMutableMapIterator<int, QByteArray> it(replies_);
		while (it.hasNext()) {
			it.next();
			if (it.key() == nextReplyId_) {
				writeReply(it.value());

				it.remove();
			} else {
				break;
			}
		}

		sock_.flush();
	} else {
		replies_[id] = reply;
	}
}

void RequestParser::detachRequest()
{
	if (!verifyThreadCall(&RequestParser::detachRequest)) return;

	if (--attachedRequests_ == 0) deleteLater();
}

void RequestParser::stateChanged(QAbstractSocket::SocketState state)
{
	if (state == QAbstractSocket::UnconnectedState) {
		socketClosed_ = true;
		logCustom(LogCat::Network | LogCat::Debug)("connection %1 closed", id_);
		detachRequest();
	}
}

void RequestParser::readyRead()
{
	const qint64 avail = sock_.bytesAvailable();
	if (avail <= 0) return;
	logCustom(LogCat::Network | LogCat::Trace)("received %1 bytes on connection %2", avail, id_);

	if (contentLength_ == -1) {
		header_ += sock_.read(avail);
	} else {
		body_ += sock_.read(avail);
	}

	do {

		// header ok?
		if (contentLength_ == -1) {
			const int pos = header_.indexOf("\r\n\r\n");
			if (pos == -1) return;

			body_ = header_.mid(pos + 4);
			header_.resize(pos);

			if (!parseHeader()) {
				sock_.abort();
				return;
			}
		}

		// body ok?
		const qint64 size = method_ == "GET" ? 0 : contentLength_;
		if (body_.size() < size) return;

		// to much bytes?
		if (body_.size() > size) {
			header_ = body_.mid(size);
			body_.resize(size);
		} else {
			header_.clear();
		}

		// notify handlers
		++attachedRequests_;
		Request(id_, ++requestCount_, headerFields_, method_, url_, body_, handlers_, this).callNextHandler();

		// reset for next
		contentLength_ = -1;
		headerFields_.clear();
		method_.clear();
		url_.clear();
		body_.clear();

	} while (!header_.isEmpty());
}

bool RequestParser::parseHeader()
{
	bool isFirst = true;
	foreach (const QByteArray & line, header_.split('\n')) {
		if (isFirst) {
			isFirst = false;
			if (!handleRequestLine(line)) return false;
			continue;
		}

		const int pos = line.indexOf(':');
		if (pos == -1) {
			logWarn("funny line in header: %1", line);
			return false;
		}
		QByteArray key = line.left(pos).trimmed();
		QByteArray value = line.mid(pos + 1).trimmed();

		headerFields_[key] = value;

		QByteArray lKey = key.toLower();
		if (lKey == "content-length") {
			bool ok;
			contentLength_ = value.toUInt(&ok);
			if (!ok) {
				logWarn("could not understand Content-Length: %1", value);
				return false;
			}
		} else if (lKey == "host") {
			const int p = value.indexOf(':');
			if (p != -1) {
				bool ok;
				quint16 port = value.mid(p + 1).toUShort(&ok);
				if (!ok) {
					logWarn("funny port in header field: %1", value.mid(p + 1));
					return false;
				}
				url_.setPort(port);
				value = value.left(p);
			}
			url_.setHost(value);
		}
	}

	if (contentLength_ == -1 && method_ != "GET") {
		logWarn("Content-Length field not found in header");
		return false;
	}

	return true;
}

bool RequestParser::handleRequestLine(const QByteArray & line)
{
	QList<QByteArray> parts = line.split(' ');
	if (parts.size() < 3) {
		logWarn("unknown request on connection %1: %2", id_, line);
		return false;
	}

	method_ = parts[0].trimmed();
	if (method_ != "GET" && method_ != "POST") {
		logWarn("unknown method on connection %1: %2", id_, method_);
		return false;
	}

	QByteArray proto = parts[2].trimmed();
	if (!proto.startsWith("HTTP/")) {
		logWarn("unknown protocol on connection %1: %2", id_, proto);
		return false;
	}

	QByteArray uri = parts[1].trimmed();
	if (uri.isEmpty()) {
		logWarn("no URI on connection %1: %2", id_);
		return false;
	}
	url_.setScheme("http");

	int pos = uri.indexOf('?');
	if (pos == -1) {
		url_.setPath(uri);
	} else {
		url_.setPath(uri.left(pos));
		url_.setQuery(uri.mid(pos + 1));
	}

	return true;
}

void RequestParser::writeReply(const QByteArray & reply)
{
	if (socketClosed_) {
		logCustom(LogCat::Network | LogCat::Info)("cannot write %1 bytes of request %2 on closed connection %3",
			reply.size(), nextReplyId_, id_);
	} else {
		sock_.write(reply);
		logCustom(LogCat::Network | LogCat::Trace)("wrote %1 bytes of request %2 on connection %3",
			reply.size(), nextReplyId_, id_);
	}
	++nextReplyId_;
}

}}}	// namespace
