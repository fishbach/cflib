/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "httprequest.h"

#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/log.h>
#include <cflib/base.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

namespace {

const CFRegex lengthRE("\r?\nContent-Length: (\\d+)\r?\n", true);
const CFRegex statusRE("^HTTP/1.[01] (\\d+)");

}

class HttpRequest::Conn : public TCPConn, public util::ThreadVerify
{
public:
    Conn(HttpRequest * parent, TCPConnData * data,
        const CFUrl & url, const CFList<ByteArray> & headers,
        const ByteArray & postData, const ByteArray & contentType,
        uint timeoutMs)
    :
        TCPConn(data),
        ThreadVerify(manager().networkThread()),
        timeout_(this, &Conn::timeout),
        parent_(parent),
        gotReply_(false)
    {
        // GET / POST
        ByteArray request = postData.isNull() ? "GET " : "POST ";

        // path
        request += url.path().isEmpty() ? ByteArray("/") : url.path().toUtf8();

        // query
        if (url.hasQuery()) request += "?" + url.query().toUtf8();
        request += " HTTP/1.0\r\n";

        // host
        request += "Host: " + url.host().toUtf8();
        if (url.port() != -1) request += ":" + ByteArray::number((cfint64)url.port());
        request += "\r\n";

        // login / password
        if (!url.userInfo().isEmpty()) {
            request += "Authorization: Basic " + url.userInfo().toUtf8().toBase64() + "\r\n";
        }

        // additional headers
        for (const ByteArray & header : headers) request += header.trimmed() + "\r\n";

        if (postData.isNull()) {
            request += "\r\n";
        } else {
            request += "Content-Length: " + ByteArray::number((cfint64)postData.size()) + "\r\n";
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
        close(HardClosed);
    }

protected:
    void newBytesAvailable() override
    {
        if (!verifyThreadCall(&Conn::newBytesAvailable)) return;

        buf_ += read();
        logTrace("received: %1", buf_);

        const int headerEndPos = buf_.indexOf("\r\n\r\n");
        if (headerEndPos == -1) {
            startReadWatcher();
            return;
        }

        CFRegex::MatchResult match = lengthRE.matchResult(buf_);
        if (!match.hasMatch()) {
            close(HardClosed, true);
            return;
        }

        const int length = ByteArray(match.captured(1).c_str()).toInt();
        if (buf_.size() < headerEndPos + 4 + length) {
            startReadWatcher();
            return;
        }

        match = statusRE.matchResult(buf_);
        if (!match.hasMatch()) {
            close(HardClosed, true);
            return;
        }

        const int status = ByteArray(match.captured(1).c_str()).toInt();

        if (parent_) parent_->reply(status, buf_.mid(headerEndPos + 4, length));
        gotReply_ = true;
        timeout_.stop();
        close(ReadWriteClosed, true);
    }

    void closed(CloseType type) override
    {
        if (!verifyThreadCall(&Conn::closed, type)) return;

        logFunctionTrace

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
    ByteArray buf_;
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

void HttpRequest::start(const CFUrl & url, const CFList<ByteArray> & headers,
    const ByteArray & postData, const ByteArray & contentType,
    uint timeoutMs)
{
    if (!verifyThreadCall(&HttpRequest::start, url, headers, postData, contentType, timeoutMs)) return;

    if (conn_) {
        logWarn("cannot handle two simultaneous requests");
        reply(429, "Too Many Requests");
        return;
    }

    TCPConnData * cd;
    if (url.scheme() == "http") {
        cd = mgr_.openConnection(
            url.host().toUtf8(),
            url.port() != -1 ? url.port() : 80);
    } else {
        cd = mgr_.openTLSConnection(
        url.host().toUtf8(),
        url.port() != -1 ? url.port() : 443);
    }

    if (!cd) {
        reply(503, "Service Unavailable");
        return;
    }
    conn_ = new Conn(this, cd, url, headers, postData, contentType, timeoutMs);
}

void HttpRequest::destroy()
{
    if (!verifySyncedThreadCall(&HttpRequest::destroy)) return;
    if (conn_) conn_->destory();
}

}}    // namespace
