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

#include "httpserver.h"

#include <cflib/net/impl/httpthread.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

class HttpServer::Impl : public TCPManager
{
public:
	Impl(uint threadCount, uint tlsThreadCount) :
		TCPManager(tlsThreadCount),
		threadCounter_(0)
	{
		for (uint i = 1 ; i <= threadCount ; ++i) threads_.append(new impl::HttpThread(i, threadCount));
	}

	~Impl()
	{
		stop();
		foreach (impl::HttpThread * th, threads_) delete th;
	}

	void registerHandler(RequestHandler & handler)
	{
		handlers_ << &handler;
	}

protected:
	virtual void newConnection(TCPConnData * data)
    {
		threads_[++threadCounter_ % threads_.size()]->newRequest(data, handlers_);
    }

private:
	QVector<impl::HttpThread *> threads_;
	uint threadCounter_;
	QList<RequestHandler *> handlers_;
};

HttpServer::HttpServer(uint threadCount, uint tlsThreadCount) :
	impl_(new Impl(threadCount, tlsThreadCount))
{
}

HttpServer::~HttpServer()
{
	delete impl_;
}

bool HttpServer::start(const QByteArray & address, quint16 port)
{
	return impl_->start(address, port);
}

bool HttpServer::start(const QByteArray & address, quint16 port, crypt::TLSCredentials & credentials)
{
	return impl_->start(address, port, credentials);
}

bool HttpServer::start(int listenSocket)
{
	return impl_->start(listenSocket);
}

bool HttpServer::start(int listenSocket, crypt::TLSCredentials & credentials)
{
	return impl_->start(listenSocket, credentials);
}

void HttpServer::stop()
{
	impl_->stop();
}

bool HttpServer::isRunning() const
{
	return impl_->isRunning();
}

void HttpServer::registerHandler(RequestHandler & handler)
{
	impl_->registerHandler(handler);
}

}}	// namespace
