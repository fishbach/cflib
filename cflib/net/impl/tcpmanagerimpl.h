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

#pragma once

#include <cflib/crypt/tlscredentials.h>
#include <cflib/net/tcpconn.h>
#include <cflib/util/threadverify.h>

struct ev_io;

namespace cflib { namespace net {

class TCPManager;

namespace impl {

class TLSThread;

class TCPManagerImpl : public util::ThreadVerify
{
public:
	TCPManagerImpl(TCPManager & parent, uint tlsThreadCount);
	TCPManagerImpl(TCPManager & parent, uint tlsThreadCount, util::ThreadVerify * other);
	~TCPManagerImpl();

	bool isRunning() const { return listenSock_ != -1; }
	bool start(int listenSocket, crypt::TLSCredentials * credentials);
	void stop();

	TCPConnData * openConnection(const QByteArray & destAddress, quint16 destPort,
		const QByteArray & sourceIP, quint16 sourcePort,
		crypt::TLSCredentials * credentials, bool preferIPv6);

	void startReadWatcher(TCPConnData * conn);
	void writeToSocket(TCPConnData * conn, const QByteArray & data, bool notifyFinished);
	void closeConn(TCPConnData * conn, TCPConn::CloseType type, bool notifyClose);
	void deleteOnFinish(TCPConnData * conn);

	void tlsStartReadWatcher(TCPConnData * conn);
	void tlsWrite(TCPConnData * conn, const QByteArray & data, bool notifyFinished) const;
	void tlsCloseConn(TCPConnData * conn, TCPConn::CloseType type, bool notifyClose) const;
	void tlsDeleteOnFinish(TCPConnData * conn) const;

	static void setNoDelay(int socket, bool noDelay);
	static int openListenSocket(const QByteArray & ip, quint16 port);

	static void readable(ev_loop * loop, ev_io * w, int revents);
	static void writeable(ev_loop * loop, ev_io * w, int revents);

	TCPManager & parent;

	crypt::TLSCredentials clientCredentials;

protected:
	virtual void deleteThreadData();

private:
	static void listenSocketReadable(ev_loop * loop, ev_io * w, int revents);
	void callClosed(TCPConnData * conn);
	TCPConnData * addConnection(int sock, const QByteArray & destIP, quint16 destPort,
		crypt::TLSCredentials * credentials, const QByteArray & destAddress);

private:
	int listenSock_;
	bool isIPv6Sock_;
	ev_io * readWatcher_;
	crypt::TLSCredentials * credentials_;
	QVector<TLSThread *> tlsThreads_;
	QAtomicInteger<uint> tlsConnId_;
	QSet<TCPConnData *> connections_;
};

}}}	// namespace
