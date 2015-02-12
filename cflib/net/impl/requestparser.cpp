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

#include <cflib/net/request.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net { namespace impl {

namespace {

QAtomicInt connCount;

}

RequestParser::RequestParser(const TCPConnInitializer * init,
	const QList<RequestHandler *> & handlers, util::ThreadVerify * tv)
:
	util::ThreadVerify(tv),
	TCPConn(init),
	handlers_(handlers),
	id_(connCount.fetchAndAddRelaxed(1) + 1),
	contentLength_(-1),
	requestCount_(0), nextReplyId_(1),
	attachedRequests_(1),
	socketClosed_(false),
	detached_(false),
	passThrough_(false),
	passThroughHandler_(0)
{
	// thread TCPServer (1/1)
	logCustom(LogCat::Network | LogCat::Debug)("new connection %1", id_);
	startWatcher();
}

RequestParser::~RequestParser()
{
	logTrace("deleted RequestParser of connection %1", id_);
}

void RequestParser::sendReply(int id, const QByteArray & reply)
{
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
	if (--attachedRequests_ == 0) deleteNext(this);
}

void RequestParser::setPassThroughHandler(PassThroughHandler * hdl)
{
	passThroughHandler_ = hdl;
}

QByteArray RequestParser::readPassThrough(bool & isLast)
{
	if (!passThrough_ || socketClosed_) {
		isLast = true;
		return QByteArray();
	}
	QByteArray retval = read();
	isLast = retval.size() >= contentLength_;
	if (isLast) {
		passThrough_ = false;
		passThroughHandler_ = 0;
		if (retval.size() > contentLength_) {
			header_ = retval.mid(contentLength_);
			retval.resize(contentLength_);
			contentLength_ = -1;
			execCall(new util::Functor0<RequestParser>(this, &RequestParser::parseRequest));
		} else {
			contentLength_ = -1;
			startWatcher();
		}
	} else {
		contentLength_ -= retval.size();
	}
	return retval;
}

const TCPConnInitializer * RequestParser::detachFromSocket()
{
	logFunctionTrace
	detached_ = true;
	detachRequest();	// removes initial ref, so that we will be deleted
	return TCPConn::detachFromSocket();
}

void RequestParser::newBytesAvailable()
{
	if (!verifyThreadCall(&RequestParser::newBytesAvailable)) return;

	if (passThrough_) {
		if (passThroughHandler_) passThroughHandler_->morePassThroughData();
		return;
	}

	QByteArray newBytes = read();
	if (newBytes.isEmpty()) {
		startWatcher();
		return;
	}
	logCustom(LogCat::Network | LogCat::Trace)("received %1 bytes on connection %2", newBytes.size(), id_);

	if (contentLength_ == -1) header_ += newBytes;
	else                      body_   += newBytes;

	parseRequest();
}

void RequestParser::closed()
{
	if (!verifyThreadCall(&RequestParser::closed)) return;

	socketClosed_ = true;
	logCustom(LogCat::Network | LogCat::Debug)("connection %1 closed", id_);
	detachRequest();	// removes initial ref, so that we will be deleted

	if (passThroughHandler_) passThroughHandler_->morePassThroughData();
}

void RequestParser::parseRequest()
{
	logFunctionTrace
	do {

		// header ok?
		if (contentLength_ == -1) {
			const int pos = header_.indexOf("\r\n\r\n");
			if (pos == -1) break;

			body_ = header_.mid(pos + 4);
			header_.resize(pos);

			if (!parseHeader()) {
				abortConnection();
				break;
			}
		}

		// body ok?
		const qint64 size = method_ == "GET" ? 0 : contentLength_;

		if (body_.size() < size) {
			// small requests we hold in memory
			if (body_.size() < 0x400000) break;
			passThrough_ = true;
		}

		// to much bytes?
		QByteArray nextHeader;
		if (body_.size() > size) {
			nextHeader = body_.mid(size);
			body_.resize(size);
		}

		// notify handlers
		++attachedRequests_;
		Request(id_, ++requestCount_, header_, headerFields_, method_, uri_, body_,
			handlers_, passThrough_, this).callNextHandler();
		if (detached_) return;

		// reset for next
		header_ = nextHeader;
		contentLength_ = passThrough_ ? (size - body_.size()) : -1;
		headerFields_.clear();
		method_.clear();
		uri_.clear();
		body_.clear();

	} while (!header_.isEmpty());

	if (!passThrough_) startWatcher();
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

	uri_ = parts[1].trimmed();
	if (uri_.isEmpty()) {
		logWarn("no URI on connection %1: %2", id_);
		return false;
	}

	QByteArray proto = parts[2].trimmed();
	if (!proto.startsWith("HTTP/")) {
		logWarn("unknown protocol on connection %1: %2", id_, proto);
		return false;
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
		closeNicely();
	}
	++nextReplyId_;
}

}}}	// namespace
