/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/base/cfconcurrent.h>
#include <cflib/net/httpclient.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/request.h>
#include <cflib/net/requesthandler.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/test.h>

#include <regex>

using namespace cflib::net;

namespace {

CFSemaphore msgSem;
CFStringList msgs;
CFMutex mutex;

void msg(const CFString & m)
{
    CFMutexLocker ml(mutex);
    msgs << m;
    msgSem.release();
}

bool msgsMatchRegex(const char * pattern)
{
    std::regex re(pattern, std::regex::ECMAScript);
    for (const auto & s : msgs) {
        if (std::regex_search(s.c_str(), re)) return true;
    }
    return false;
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
            request.sendText("reply " + CFString::number(++count_));
        }
    }
private:
    uint count_;
};

class TestClient : public HttpClient
{
public:
    TestClient(TCPManager & mgr, bool keepAlive) : HttpClient(mgr, keepAlive) {}

protected:
    virtual void reply(const CFByteArray & raw)
    {
        CFByteArray r = raw;
        r.replace("\r\n", "|");
        msg("http reply: " + r);
    }
};

}

class HTTP_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<HTTP_Test *>(this);
        return {
            {"test_keepAlive",      [self]() { self->test_keepAlive(); }},
            {"test_immediateClose", [self]() { self->test_immediateClose(); }}
        };
    }

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
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "new request: /test1"));
        QVERIFY(msgsMatchRegex(
            "http reply: HTTP/1\\.1 200 OK\\|"
            "Date: .*\\|"
            "Server: cflib/.*\\|"
            "Connection: keep-alive\\|"
            "Content-Type: text/html; charset=utf-8|Content-Length: 7||reply 1"
        ));
        msgs.clear();

        cli.get("127.0.0.1", 12301, "/test2");
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "new request: /test2"));
        QVERIFY(msgsMatchRegex(
            "http reply: HTTP/1\\.1 200 OK\\|"
            "Date: .*\\|"
            "Server: cflib/.*\\|"
            "Connection: keep-alive\\|"
            "Content-Type: text/html; charset=utf-8|Content-Length: 7||reply 2"
        ));
        msgs.clear();

        cli.get("127.0.0.1", 12301, "/abort");
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "new request: /abort"));
        QVERIFY(cfContains(msgs, "http reply: "));
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
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "new request: /test1"));
        QVERIFY(msgsMatchRegex(
            "http reply: HTTP/1\\.1 200 OK\\|"
            "Date: .*\\|"
            "Server: cflib/.*\\|"
            "Connection: keep-alive\\|"
            "Content-Type: text/html; charset=utf-8|Content-Length: 7||reply 1"
        ));
        msgs.clear();

        cli.get("127.0.0.1", 12301, "/test2");
        msgSem.acquire(2);
        QCOMPARE((int)msgs.size(), 2);
        QVERIFY(cfContains(msgs, "new request: /test2"));
        QVERIFY(msgsMatchRegex(
            "http reply: HTTP/1\\.1 200 OK\\|"
            "Date: .*\\|"
            "Server: cflib/.*\\|"
            "Connection: keep-alive\\|"
            "Content-Type: text/html; charset=utf-8|Content-Length: 7||reply 2"
        ));
        msgs.clear();
    }
};

ADD_TEST(HTTP_Test)
