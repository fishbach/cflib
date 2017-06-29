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

#pragma once

#include <cflib/util/sig.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class TCPManager;

class HttpRequest : public util::ThreadVerify
{
	Q_DISABLE_COPY(HttpRequest)
public:
	HttpRequest(TCPManager & mgr);
	~HttpRequest();

	void start(const QUrl & url, const QList<QByteArray> & headers,
		const QByteArray & postData = QByteArray(), const QByteArray & contentType = "application/octet-stream",
		uint timeoutMs = 10000);

	inline void start(const QUrl & url,
		const QByteArray & postData = QByteArray(), const QByteArray & contentType = "application/octet-stream",
		uint timeoutMs = 10000)
	{
		start(url, QList<QByteArray>(), postData, contentType, timeoutMs);
	}

cfsignals:
	sig<void (int status, const QByteArray & reply)> reply;

private:
	void destroy();

private:
	TCPManager & mgr_;
	class Conn;
	Conn * conn_;
};

}}	// namespace
