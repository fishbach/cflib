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

#include "httpclient.h"

#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

class HttpClient::HttpConn : public TCPConn
{
public:
	HttpConn(TCPConnData * data, HttpClient & parent) :
		TCPConn(data),
		parent_(parent)
	{
	}

protected:
	virtual void newBytesAvailable()
	{
		parent_.reply(read());
		if (!parent_.keepAlive_) {
			parent_.conn_ = 0;
			delete this;
		}
	}

	virtual void closed(CloseType)
	{
		parent_.reply(QByteArray());
		if (!parent_.keepAlive_) {
			delete this;
		}
	}

private:
	HttpClient & parent_;
};


HttpClient::HttpClient(TCPManager & mgr, bool keepAlive) :
	mgr_(mgr), keepAlive_(keepAlive), conn_(0)
{
}

HttpClient::~HttpClient()
{
	delete conn_;
}

void HttpClient::get(const QByteArray & ip, quint16 port, const QByteArray & url)
{
	if (!conn_) {
		TCPConnData * data = mgr_.openConnection(ip, port);
		if (!data) {
			reply(QByteArray());
			return;
		}
		conn_ = new HttpConn(data, *this);
	}

	QByteArray request;
	request <<
		"GET " << url << " HTTP/1.1\r\n"
		"Host: " << ip << ":" << QByteArray::number(port) << "\r\n"
		<< (keepAlive_ ? "Connection: keep-alive\r\n" : "") <<
		"\r\n";
	conn_->write(request);
	if (!keepAlive_) conn_->close(TCPConn::WriteClosed);
	conn_->startReadWatcher();
	if (!keepAlive_) conn_ = 0;
}

}}	// namespace
