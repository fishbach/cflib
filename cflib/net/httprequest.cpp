/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include "httprequest.h"

#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

namespace {

const QRegularExpression lengthRE("\r?\nContent-Length: (\\d+)\r?\n", QRegularExpression::CaseInsensitiveOption);
const QRegularExpression statusRE("^HTTP/1.[01] (\\d+)");

}

class HttpRequest::Conn : public TCPConn
{
public:
	Conn(HttpRequest * parent, TCPConnData * data,
		const QUrl & url,
		const QByteArray & postData, const QByteArray & contentType,
		uint timeoutMs)
	:
		TCPConn(data),
		timeout_(this, &Conn::timeout),
		parent_(parent),
		gotReply_(false)
	{
		// GET / POST
		QByteArray request = postData.isNull() ? "GET " : "POST ";

		// path
		request += url.path().isEmpty() ? "/" : url.path(QUrl::FullyEncoded).toUtf8();

		// query
		if (url.hasQuery()) request += "?" + url.query(QUrl::FullyEncoded).toUtf8();
		request += " HTTP/1.0\r\n";

		// host
		request += "Host: " + url.host(QUrl::FullyEncoded).toUtf8();
		if (url.port() != -1) request += ":" + QByteArray::number(url.port());
		request += "\r\n";

		// login / password
		if (!url.userInfo().isEmpty()) {
			request += "Authorization: Basic " + url.userInfo().toUtf8().toBase64() + "\r\n";
		}

		if (postData.isNull()) {
			request += "\r\n";
		} else {
			request += "Content-Length: " + QByteArray::number(postData.size()) + "\r\n";
			request += "Content-Type: " + contentType + "\r\n";
			request += "\r\n";
			request += postData;
		}

		logTrace("sending: %1", request);
		write(request);

		startReadWatcher();

		timeout_.start(timeoutMs / 1000.0, 0.0);
	}

	~Conn()
	{
		logFunctionTrace
	}

	void destory()
	{
		parent_ = 0;
		close(HardClosed, true);
	}

protected:
	void newBytesAvailable() override
	{
		buf_ += read();
		logTrace("received: %1", buf_);

		const int headerEndPos = buf_.indexOf("\r\n\r\n");
		if (headerEndPos == -1) {
			startReadWatcher();
			return;
		}

		QRegularExpressionMatch match = lengthRE.match(buf_);
		if (!match.hasMatch()) {
			close(HardClosed, true);
			return;
		}

		const int length = match.captured(1).toInt();
		if (buf_.size() < headerEndPos + 4 + length) {
			startReadWatcher();
			return;
		}

		match = statusRE.match(buf_);
		if (!match.hasMatch()) {
			close(HardClosed, true);
			return;
		}

		const int status = match.captured(1).toInt();

		if (parent_) parent_->reply(status, buf_.mid(headerEndPos + 4, length));
		gotReply_ = true;
		timeout_.stop();
		close();
	}

	void closed(CloseType) override
	{
		if (parent_) {
			if (!gotReply_) parent_->reply(503, "Service Unavailable");
			parent_->conn_ = 0;
		}
		timeout_.stop();
		util::deleteNext(this);
	}

private:
	void timeout()
	{
		logFunctionTrace

		if (parent_) {
			parent_->reply(408, "Request Timeout");
			gotReply_ = true;
		}
		close(HardClosed, true);
	}

private:
	util::EVTimer timeout_;
	HttpRequest * parent_;
	bool gotReply_;
	QByteArray buf_;
};

HttpRequest::HttpRequest(TCPManager & mgr) :
	ThreadVerify(mgr.networkThread()),
	mgr_(mgr),
	conn_(0)
{
}

HttpRequest::~HttpRequest()
{
	logFunctionTrace
	destroy();
}

void HttpRequest::start(const QUrl & url,
	const QByteArray & postData, const QByteArray & contentType,
	uint timeoutMs)
{
	if (!verifyThreadCall(&HttpRequest::start, url, postData, contentType, timeoutMs)) return;

	if (conn_) {
		logWarn("cannot handle to simultaneous requests");
		reply(429, "Too Many Requests");
		return;
	}

	TCPConnData * cd = mgr_.openConnection(
		url.host(QUrl::FullyEncoded).toUtf8(),
		url.port() != -1 ? url.port() : url.scheme() == "http" ? 80 : 443);
	if (!cd) {
		reply(503, "Service Unavailable");
		return;
	}
	conn_ = new Conn(this, cd, url, postData, contentType, timeoutMs);
}

void HttpRequest::destroy()
{
	if (!verifySyncedThreadCall(&HttpRequest::destroy)) return;
	if (conn_) conn_->destory();
}

}}	// namespace