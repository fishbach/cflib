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

#include <cflib/net/httpclient.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/request.h>
#include <cflib/net/requesthandler.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/test.h>

using namespace cflib::net;

namespace {

QSemaphore msgSem;
QStringList msgs;
QMutex mutex;

void msg(const QString & msg)
{
	QMutexLocker ml(&mutex);
	msgs << msg;
	msgSem.release();
}

class TestHdl : public RequestHandler
{
public:
	TestHdl() : count_(0) {}

protected:
	virtual void handleRequest(const Request & request)
	{
		msg("new request: " + request.getUri());
		if (request.getUri() == "/abort") {
			request.abort();
		} else {
			request.sendText(QString("reply %1").arg(++count_));
		}
	}
private:
	uint count_;
};

class TestClient : public HttpClient
{
public:
	TestClient(TCPManager & mgr,  bool keepAlive) : HttpClient(mgr, keepAlive) {}

protected:
	virtual void reply(const QByteArray & raw)
	{
		QByteArray r = raw;
		r.replace("\r\n" , "|");
		msg("http reply: " + r);
	}

};

}

class HTTP_Test: public QObject
{
	Q_OBJECT
private slots:

	void test_keepAlive()
	{
		TestHdl hdl;
		HttpServer server;
		server.registerHandler(hdl);
		server.start("127.0.0.1", 12301);

		TCPManager mgr;
		TestClient cli(mgr, true);

		cli.get("127.0.0.1", 12301, "/test1");
		msgSem.acquire(2);
		QCOMPARE(msgs.size(), 2);
		QVERIFY(msgs.contains("new request: /test1"));
		QVERIFY(!msgs.filter(QRegularExpression(
			"http reply: HTTP/1.1 200 OK\\|"
			"Date: .*\\|"
			"Server: cflib/.*\\|"
			"Connection: keep-alive\\|"
			"Content-Type: text/html; charset=utf-8|Content-Length: 7||reply 1"
		)).isEmpty());
		msgs.clear();

		cli.get("127.0.0.1", 12301, "/test2");
		msgSem.acquire(2);
		QCOMPARE(msgs.size(), 2);
		QVERIFY(msgs.contains("new request: /test2"));
		QVERIFY(!msgs.filter(QRegularExpression(
			"http reply: HTTP/1.1 200 OK\\|"
			"Date: .*\\|"
			"Server: cflib/.*\\|"
			"Connection: keep-alive\\|"
			"Content-Type: text/html; charset=utf-8|Content-Length: 7||reply 2"
		)).isEmpty());
		msgs.clear();


		cli.get("127.0.0.1", 12301, "/abort");
		msgSem.acquire(2);
		QCOMPARE(msgs.size(), 2);
		QVERIFY(msgs.contains("new request: /abort"));
		QVERIFY(msgs.contains("http reply: "));
		msgs.clear();
	}

	void test_immediateClose()
	{
		TestHdl hdl;

		HttpServer server;
		server.registerHandler(hdl);
		server.start("127.0.0.1", 12301);

		TCPManager mgr;
		TestClient cli(mgr, false);

		cli.get("127.0.0.1", 12301, "/test1");
		msgSem.acquire(2);
		QCOMPARE(msgs.size(), 2);
		QVERIFY(msgs.contains("new request: /test1"));
		QVERIFY(!msgs.filter(QRegularExpression(
			"http reply: HTTP/1.1 200 OK\\|"
			"Date: .*\\|"
			"Server: cflib/.*\\|"
			"Connection: keep-alive\\|"
			"Content-Type: text/html; charset=utf-8|Content-Length: 7||reply 1"
		)).isEmpty());
		msgs.clear();

		cli.get("127.0.0.1", 12301, "/test2");
		msgSem.acquire(2);
		QCOMPARE(msgs.size(), 2);
		QVERIFY(msgs.contains("new request: /test2"));
		QVERIFY(!msgs.filter(QRegularExpression(
			"http reply: HTTP/1.1 200 OK\\|"
			"Date: .*\\|"
			"Server: cflib/.*\\|"
			"Connection: keep-alive\\|"
			"Content-Type: text/html; charset=utf-8|Content-Length: 7||reply 2"
		)).isEmpty());
		msgs.clear();
	}

};
#include "http_test.moc"
ADD_TEST(HTTP_Test)
