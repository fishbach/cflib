/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "request.h"

#include <cflib/base.h>

#include <cflib/net/requesthandler.h>
#include <cflib/net/impl/requestparser.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

class Request::Shared
{
public:
    Shared(int connId, int requestId,
        const ByteArray & header,
        const Request::KeyVal & headerFields, Request::Method method, const ByteArray & uri,
        const ByteArray & body, const CFList<RequestHandler *> & handlers, bool passThrough,
        impl::RequestParser * parser)
    :
        ref(1),
        connId(connId),
        requestId(requestId),
        header(header),
        headerFields(headerFields), method(method), uri(uri), body(body),
        handlers(handlers),
        parser(parser),
        replySent(parser == 0),
        passThrough(passThrough),
        detached(false)
    {
        if (headerFields.contains("x-remote-ip")) {
            remoteIP = headerFields.count("x-remote-ip") ? headerFields.at("x-remote-ip") : ByteArray();
        } else if (parser) {
            remoteIP = parser->peerIP();
        }
        watch.start();
        id << ByteArray::number(connId) << '-' << ByteArray::number(requestId);
        logDebug("new request %1 (body len: %2)", id, headerFields.count("content-length") ? headerFields.at("content-length") : ByteArray());
    }

    ~Shared()
    {
        const int msec = watch.elapsed();
        if (detached) {
            logDebug("request %1 detached", id);
        } else if (!replySent) {
            sendNotFound();
            logDebug("request %1 finished with 404 (msec: %2)", id, msec);
        } else {
            logDebug("request %1 finished successfully (msec: %2)", id, msec);
        }

        if (parser) parser->detachRequest();
    }

    CFAtomicInt ref;
    int connId;
    int requestId;
    ByteArray id;
    ByteArray header;
    Request::KeyVal headerFields;
    Request::Method method;
    ByteArray uri;
    ByteArray body;
    CFList<RequestHandler *> handlers;
    impl::RequestParser * parser;
    CFElapsedTimer watch;
    bool replySent;
    ByteArray remoteIP;
    CFList<ByteArray> sendHeaderLines;
    bool passThrough;
    bool detached;

public:
    ByteArray getRequestField(const ByteArray & key)
    {
        const ByteArray search = key.toLower();
        for (const auto & [k, v] : headerFields) {
            if (k.toLower() == search) return v;
        }
        return ByteArray();
    }

    void sendReply(ByteArray header, ByteArray body, bool compression)
    {
        if (replySent) {
            logWarn("tried to send two replies for request %1", id);
            return;
        }
        replySent = true;

        // compression
        if (compression && method != Request::HEAD && body.size() > 256 &&
            getRequestField("Accept-Encoding").indexOf("gzip") != -1)
        {
            header += "Content-Encoding: gzip\r\n";
            cflib::util::gzip(body, 1);
        }

        if (method != Request::HEAD) {
            header
                << "Content-Length: " << ByteArray::number((cfint64)body.size()) << "\r\n"
                << "\r\n"
                << body;
        } else {
            header += "\r\n";
        }

        parser->sendReply(requestId, header);
    }

    void sendNotFound()
    {
        ByteArray hdr = "HTTP/1.1 404 Not Found\r\n";
        hdr << defaultHeaders() << "Content-Type: text/html; charset=utf-8\r\n";
        sendReply(hdr,
            "<html>\r\n"
            "<head><title>404 - Not Found</title></head>\r\n"
            "<body>\r\n"
            "<h1>404 - Not Found</h1>\r\n"
            "</body>\r\n"
            "</html>\r\n",
            false);
    }

    inline ByteArray defaultHeaders()
    {
        ByteArray headers = "Date: ";
        headers << cflib::util::dateTimeForHTTP(CFDateTime::currentDateTimeUtc()) << "\r\n"
            "Connection: keep-alive\r\n"
            "Server: cflib/0.9\r\n";
        for (const auto & line : sendHeaderLines) headers << line << "\r\n";
        return headers;
    }

};

Request::Request() :
    d(new Shared(0, 0, ByteArray(), KeyVal(), NONE, ByteArray(), ByteArray(), CFList<RequestHandler *>(), false, 0))
{
}

Request::Request(int connId, int requestId,
    const ByteArray & header,
    const KeyVal & headerFields, Method method, const ByteArray & uri,
    const ByteArray & body, const CFList<RequestHandler *> & handlers, bool passThrough,
    impl::RequestParser * parser)
:
    d(new Shared(connId, requestId, header, headerFields, method, uri, body, handlers, passThrough, parser))
{
}

Request::~Request()
{
    while (!d->ref.deref()) {
        if (d->replySent || d->handlers.empty()) {
            logTrace("request deleted");
            delete d;
            return;
        }
        d->ref.ref();
        callNextHandler();
    }
}

Request::Request(const Request & other) :
    d(other.d)
{
    d->ref.ref();
}

Request & Request::operator=(const Request & other)
{
    if (d != other.d) {
        other.d->ref.ref();
        if (!d->ref.deref()) delete d;
        d = other.d;
    }
    return *this;
}

Request::Id Request::getId() const
{
    return CFPair(d->connId, d->requestId);
}

bool Request::replySent() const
{
    return d->replySent;
}

ByteArray Request::getRawHeader() const
{
    return d->header;
}

ByteArray Request::getHeader(const ByteArray & name) const
{
    return cfMapValue(d->headerFields, name);
}

ByteArray Request::getHostname() const
{
    return cfMapValue(d->headerFields, ByteArray("host"));
}

Request::KeyVal Request::getHeaderFields() const
{
    return d->headerFields;
}

Request::Method Request::getMethod() const
{
    return d->method;
}

ByteArray Request::getMethodName() const
{
    switch (d->method) {
        case NONE: return "-";
        case GET:  return "GET";
        case POST: return "POST";
        case HEAD: return "HEAD";
    }
    return ByteArray();
}

ByteArray Request::getUri() const
{
    return d->uri;
}

ByteArray Request::getBody() const
{
    return d->body;
}

ByteArray Request::getRemoteIP() const
{
    return d->remoteIP;
}

Request::LoginPass Request::getBasicAuth() const
{
    return getBasicAuth(cfMapValue(d->headerFields, ByteArray("authorization")));
}

void Request::sendNotFound() const
{
    d->sendNotFound();
}

void Request::sendRedirect(const ByteArray & url) const
{
    ByteArray hdr = "HTTP/1.1 307 Temporary Redirect\r\n"
        "Location: ";
    hdr << url << "\r\n" << d->defaultHeaders() << "Content-Type: text/html; charset=utf-8\r\n";
    d->sendReply(hdr,
        "<html>\r\n"
        "<head><title>307 - Temporary Redirect</title></head>\r\n"
        "<body>\r\n"
        "<h1>307 - Temporary Redirect</h1>\r\n"
        "</body>\r\n"
        "</html>\r\n",
        false);
}

void Request::sendReply(const ByteArray & reply, const ByteArray & contentType, bool compression) const
{
    ByteArray hdr = "HTTP/1.1 200 OK\r\n";
    hdr << d->defaultHeaders() << "Content-Type: " << contentType << "\r\n";
    d->sendReply(hdr, reply, compression);
}

void Request::sendText(const String & reply, const ByteArray & contentType, bool compression) const
{
    sendReply(reply.toUtf8(), contentType + "; charset=utf-8", compression);
}

void Request::sendRaw(const ByteArray & header, const ByteArray & body, bool compression) const
{
    d->sendReply(header, body, compression);
}

void Request::addHeaderLine(const ByteArray & line) const
{
    d->sendHeaderLines << line;
}

ByteArray Request::defaultHeaders() const
{
    return d->defaultHeaders();
}

bool Request::isPassThrough() const
{
    return d->passThrough;
}

void Request::setPassThroughHandler(PassThroughHandler * hdl) const
{
    if (d->parser) d->parser->setPassThroughHandler(hdl);
}

ByteArray Request::readPassThrough(bool & isLast) const
{
    if (!d->parser) return ByteArray();
    return d->parser->readPassThrough(isLast);
}

void Request::startWatcher() const
{
    if (d->parser) d->parser->startReadWatcher();
}

void Request::abort() const
{
    if (d->parser) d->parser->close(TCPConn::HardClosed);
}

TCPConnData * Request::detach() const
{
    if (!d->parser) return 0;
    d->replySent = true;
    d->detached = true;
    return d->parser->detach();
}

TCPManager * Request::tcpManager() const
{
    if (!d->parser) return 0;
    return &d->parser->manager();
}

Request::LoginPass Request::getBasicAuth(const ByteArray & authorization)
{
    static const CFRegex authRe("^Basic\\s+([A-Za-z0-9+/]+=*)$");

    const CFRegex::MatchResult match = authRe.matchResult(authorization);
    if (!match.hasMatch()) return LoginPass();
    const CFList<ByteArray> userPass = ByteArray::fromBase64(ByteArray(match.captured(1).c_str())).split(':');
    if (userPass.size() != 2) return LoginPass();
    return { userPass[0], userPass[1] };
}

void Request::callNextHandler() const
{
    { auto * hdl = d->handlers.front(); d->handlers.erase(d->handlers.begin()); hdl->handleRequest(*this); }
}

}}    // namespace
