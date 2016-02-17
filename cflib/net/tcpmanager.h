/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

#include <QtCore>

namespace cflib { namespace crypt { class TLSCredentials; }}
namespace cflib { namespace util  { class ThreadVerify; }}

namespace cflib { namespace net {

class TCPConnData;
namespace impl { class TCPManagerImpl; }

class TCPManager
{
	Q_DISABLE_COPY(TCPManager)
public:
	TCPManager(uint tlsThreadCount = 0);	// must be set > 0 when TLS is used
	virtual ~TCPManager();

	bool start(const QByteArray & ip, quint16 port) { return start(openListenSocket(ip, port)); }
	bool start(const QByteArray & ip, quint16 port, crypt::TLSCredentials & credentials) {
		return start(openListenSocket(ip, port), credentials); }
	void stop();
	bool isRunning() const;

	TCPConnData * openConnection(const QByteArray & destIP, quint16 destPort);
	TCPConnData * openConnection(const QByteArray & destIP, quint16 destPort,
		crypt::TLSCredentials & credentials);
	TCPConnData * openConnection(const QByteArray & destIP, quint16 destPort,
		const QByteArray & sourceIP, quint16 sourcePort);
	TCPConnData * openConnection(const QByteArray & destIP, quint16 destPort,
		const QByteArray & sourceIP, quint16 sourcePort,
		crypt::TLSCredentials & credentials);

	static int openListenSocket(const QByteArray & ip, quint16 port);
	bool start(int listenSocket);
	bool start(int listenSocket, crypt::TLSCredentials & credentials);

	util::ThreadVerify * networkThread();

protected:
	virtual void newConnection(TCPConnData * data);

private:
	impl::TCPManagerImpl * impl_;
	friend class impl::TCPManagerImpl;
};

}}	// namespace
