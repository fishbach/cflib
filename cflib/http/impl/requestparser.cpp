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

RequestParser::RequestParser(const util::TCPServer::ConnInitializer & init,
	const QList<RequestHandler *> & handlers, util::ThreadVerify * tv)
:
	util::ThreadVerify(tv),
	util::TCPConn(init),
	handlers_(handlers),
	id_(connCount.fetchAndAddRelaxed(1) + 1),
	contentLength_(-1),
	requestCount_(0), nextReplyId_(1),
	attachedRequests_(1),
	socketClosed_(false),
	passThrough_(false),
	passThroughHandler_(0)
{
	logCustom(LogCat::Network | LogCat::Debug)("new connection %1", id_);

	newBytesAvailable();
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
	} else {
		replies_[id] = reply;
	}
}

void RequestParser::detachRequest()
{
	if (!verifyThreadCall(&RequestParser::detachRequest)) return;

	if (--attachedRequests_ == 0) deleteNext();
}

void RequestParser::setPassThroughHandler(PassThroughHandler * hdl)
{
	if (!verifyThreadCall(&RequestParser::setPassThroughHandler, hdl)) return;

	passThroughHandler_ = hdl;
}

QByteArray RequestParser::readPassThrough(bool & isLast)
{
	SyncedThreadCall<QByteArray> stc(this);
	if (!stc.verify(&RequestParser::readPassThrough, isLast)) return stc.retval();

	if (!passThrough_) return QByteArray();
	QByteArray retval = read();
	isLast = retval.size() >= contentLength_;
	if (isLast) {
		bool hasMore = retval.size() > contentLength_;
		if (hasMore) {
			header_ += retval.mid(contentLength_);
			retval.resize(contentLength_);
		}
		contentLength_ = -1;
		passThrough_ = false;
		if (hasMore) newBytesAvailable();
	} else {
		contentLength_ -= retval.size();
	}
	return retval;
}

void RequestParser::newBytesAvailable()
{
	if (!verifyThreadCall(&RequestParser::newBytesAvailable)) return;

	if (passThrough_) {
		if (passThroughHandler_) passThroughHandler_->morePassThroughData(qMakePair(id_, requestCount_));
		return;
	}

	{
		QByteArray newBytes = read();
		logCustom(LogCat::Network | LogCat::Trace)("received %1 bytes on connection %2", newBytes.size(), id_);

		if (contentLength_ == -1) header_ += newBytes;
		else                      body_   += newBytes;
	}

	do {

		// header ok?
		if (contentLength_ == -1) {
			const int pos = header_.indexOf("\r\n\r\n");
			if (pos == -1) return;

			body_ = header_.mid(pos + 4);
			header_.resize(pos);

			if (!parseHeader()) {
				close();
				return;
			}
		}

		// body ok?
		const qint64 size = method_ == "GET" ? 0 : contentLength_;

		if (body_.size() < size) {
			// small requests we hold in memory
			if (size <= 8192) return;
			passThrough_ = true;
		}

		// to much bytes?
		if (body_.size() > size) {
			header_ = body_.mid(size);
			body_.resize(size);
		} else {
			header_.clear();
		}

		// notify handlers
		++attachedRequests_;
		Request(id_, ++requestCount_, headerFields_, method_, url_, body_, handlers_, passThrough_, this).callNextHandler();

		// reset for next
		contentLength_ = passThrough_ ? (size - body_.size()) : -1;
		headerFields_.clear();
		method_.clear();
		url_.clear();
		body_.clear();

	} while (!header_.isEmpty());
}

void RequestParser::closed()
{
	if (!verifyThreadCall(&RequestParser::closed)) return;

	socketClosed_ = true;
	logCustom(LogCat::Network | LogCat::Debug)("connection %1 closed", id_);
	detachRequest();
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
		QByteArray key   = line.left(pos).trimmed().toLower();
		QByteArray value = line.mid(pos + 1).trimmed();

		headerFields_[key] = value;

		if (key == "content-length") {
			bool ok;
			contentLength_ = value.toULongLong(&ok);
			if (!ok) {
				logWarn("could not understand Content-Length: %1", value);
				return false;
			}
		} else if (key == "host") {
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
		write(reply);
		logCustom(LogCat::Network | LogCat::Trace)("wrote %1 bytes of request %2 on connection %3",
			reply.size(), nextReplyId_, id_);
	}
	if (passThrough_) {
		logCustom(LogCat::Network | LogCat::Debug)("Not all bytes from pass through read! Closing connection %1 of request %2",
			id_, nextReplyId_);
		close();
	}
	++nextReplyId_;
}

}}}	// namespace
