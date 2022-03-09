/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

#include <cflib/net/impl/httpthread.h>
#include <cflib/net/request.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net { namespace impl {

namespace {

QAtomicInt connCount;

}

RequestParser::RequestParser(TCPConnData * data,
	const QList<RequestHandler *> & handlers, HttpThread * thread)
:
	util::ThreadVerify(thread),
	TCPConn(data),
	handlers_(handlers),
	thread_(thread),
	id_(connCount.fetchAndAddRelaxed(1) + 1),
	contentLength_(-1),
	method_(Request::NONE),
	requestCount_(0), nextReplyId_(1),
	attachedRequests_(1),
	detached_(false),
	passThrough_(false),
	passThroughHandler_(0)
{
	// thread TCPManager (1/1)
	logCustom(LogCat::Network | LogCat::Debug)("new connection %1", id_);
	startReadWatcher();
}

RequestParser::~RequestParser()
{
	logTrace("deleted RequestParser of connection %1", id_);
	thread_->requestFinished();
}

void RequestParser::sendReply(int id, const QByteArray & reply)
{
	if (!verifyThreadCall(&RequestParser::sendReply, id, reply)) return;

	if (id == nextReplyId_) {
		writeReply(reply);

		QMutableMapIterator<int, QByteArray> it(replies_);
		while (it.hasNext()) {
			it.next();
			if (it.key() != nextReplyId_) break;
			writeReply(it.value());
			it.remove();
		}
	} else {
		replies_[id] = reply;
	}
}

void RequestParser::detachRequest()
{
	if (!verifyThreadCall(&RequestParser::detachRequest)) return;

	if (--attachedRequests_ == 0) util::deleteNext(this);
}

void RequestParser::setPassThroughHandler(PassThroughHandler * hdl)
{
	passThroughHandler_ = hdl;
}

QByteArray RequestParser::readPassThrough(bool & isLast)
{
	if (!passThrough_ || (isClosed() & ReadClosed)) {
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
			execLater(new util::Functor0<RequestParser>(this, &RequestParser::parseRequest));
		} else {
			contentLength_ = -1;
			startReadWatcher();
		}
	} else {
		contentLength_ -= retval.size();
	}
	return retval;
}

TCPConnData * RequestParser::detach()
{
	logFunctionTrace
	detached_ = true;
	detachRequest();	// removes initial ref, so that we will be deleted
	return TCPConn::detach();
}

void RequestParser::newBytesAvailable()
{
	if (!verifyThreadCall(&RequestParser::newBytesAvailable)) return;

	if (passThrough_) {
		if (passThroughHandler_) passThroughHandler_->morePassThroughData();
		return;
	}

	QByteArray newBytes = read();
	logCustom(LogCat::Network | LogCat::Trace)("received %1 bytes on connection %2", newBytes.size(), id_);

	if (contentLength_ == -1) header_ += newBytes;
	else                      body_   += newBytes;

	parseRequest();
}

void RequestParser::closed(CloseType type)
{
	if (!verifyThreadCall(&RequestParser::closed, type)) return;

	logCustom(LogCat::Network | LogCat::Debug)("connection %1 closed (type: %2)", id_, (int)type);
	detachRequest();	// removes initial ref, so that we will be deleted

	if (passThroughHandler_) passThroughHandler_->morePassThroughData();
}

void RequestParser::parseRequest()
{
	do {

		// header finished?
		if (contentLength_ == -1) {
			const int pos = header_.indexOf("\r\n\r\n");
			if (pos == -1) break;

			body_ = header_.mid(pos + 4);
			header_.resize(pos);

			if (!parseHeader()) {
				close(HardClosed);
				break;
			}
		}

		// body ok?
		const qint64 size = method_ == Request::POST ? contentLength_ : 0;

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
		Request(id_, ++requestCount_, header_, headerFields_, (Request::Method)method_, uri_, body_,
			handlers_, passThrough_, this).callNextHandler();
		if (detached_) return;

		// reset for next
		header_ = nextHeader;
		contentLength_ = passThrough_ ? (size - body_.size()) : -1;
		headerFields_.clear();
		method_ = Request::NONE;
		uri_.clear();
		body_.clear();

	} while (!header_.isEmpty());

	if (!passThrough_) startReadWatcher();
}

bool RequestParser::parseHeader()
{
	bool isFirst = true;
	int start = 0;
	int end = header_.indexOf("\r\n", 0);
	if (end < 0) end = header_.size();
	while (start < end) {
		const QByteArray line = header_.mid(start, end - start);
		start = end + 2;
		end = header_.indexOf("\r\n", start);
		if (end < 0) end = header_.size();

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
		QByteArray key   = line.left(pos).toLower();
		QByteArray value = line.mid(pos + (line[pos + 1] == ' ' ? 2 : 1));

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

	if (contentLength_ == -1 && method_ == Request::POST) {
		logWarn("Content-Length field not found in header");
		return false;
	}

	return true;
}

bool RequestParser::handleRequestLine(const QByteArray & line)
{
	QList<QByteArray> parts = line.split(' ');
	if (parts.size() != 3) {
		logWarn("unknown request on connection %1: %2", id_, line);
		return false;
	}

	const QByteArray & method = parts[0];
	if      (method == "GET" ) method_ = Request::GET;
	else if (method == "POST") method_ = Request::POST;
	else if (method == "HEAD") method_ = Request::HEAD;
	else {
		logWarn("unknown method on connection %1: %2", id_, line);
		return false;
	}

	uri_ = parts[1];
	if (uri_.isEmpty()) {
		logWarn("no URI on connection %1: %2", id_, line);
		return false;
	}

	const QByteArray & proto = parts[2];
	if (!proto.startsWith("HTTP/")) {
		logWarn("unknown protocol on connection %1: %2", id_, line);
		return false;
	}

	return true;
}

void RequestParser::writeReply(const QByteArray & reply)
{
	if (isClosed() & WriteClosed) {
		logCustom(LogCat::Network | LogCat::Warn)("cannot write %1 bytes of request %2 on closed connection %3",
			reply.size(), nextReplyId_, id_);
	} else {
		write(reply);
		logCustom(LogCat::Network | LogCat::Trace)("wrote %1 bytes of request %2 on connection %3",
			reply.size(), nextReplyId_, id_);
	}
	if (passThrough_) {
		logCustom(LogCat::Network | LogCat::Warn)("Not all bytes from pass through read! Closing connection %1 of request %2",
			id_, nextReplyId_);
		close(ReadWriteClosed);
	}
	++nextReplyId_;
}

}}}	// namespace
