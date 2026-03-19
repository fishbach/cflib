/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::net {

class PassThroughHandler;
class RequestHandler;
class TCPConnData;
class TCPManager;
namespace impl { class RequestParser; }

class Request
{
public:
    typedef Pair<int, int> Id;
    typedef Map<ByteArray, ByteArray> KeyVal;
    enum Method {
        NONE = 0,
        GET,
        POST,
        HEAD
    };
    struct LoginPass { String login; String password; };

public:
    Request();
    Request(int connId, int requestId,
        const ByteArray & header,
        const KeyVal & headerFields, Method method, const ByteArray & uri,
        const ByteArray & body, const List<RequestHandler *> & handlers, bool passThrough,
        impl::RequestParser * parser);

    // implicit sharing
    ~Request();
    Request(const Request & other);
    Request & operator=(const Request & other);

    Id getId() const;
    bool replySent() const;

    ByteArray getRawHeader() const;
    ByteArray getHeader(const ByteArray & name) const;
    ByteArray getHostname() const;
    KeyVal getHeaderFields() const;
    Method getMethod() const;
    ByteArray getMethodName() const;
    inline bool isGET()  const { return getMethod() == GET; }
    inline bool isPOST() const { return getMethod() == POST; }
    inline bool isHEAD() const { return getMethod() == HEAD; }
    ByteArray getUri() const;
    ByteArray getBody() const;
    ByteArray getRemoteIP() const;
    LoginPass getBasicAuth() const;

    void sendNotFound() const;
    void sendRedirect(const ByteArray & url) const;
    void sendReply(const ByteArray & reply, const ByteArray & contentType, bool compression = true) const;
    void sendText(const String & reply, const ByteArray & contentType = "text/html", bool compression = true) const;
    void sendRaw(const ByteArray & header, const ByteArray & body, bool compression) const;
    void addHeaderLine(const ByteArray & line) const;
    ByteArray defaultHeaders() const;

    bool isPassThrough() const;
    void setPassThroughHandler(PassThroughHandler * hdl) const;
    ByteArray readPassThrough(bool & isLast) const;
    void startWatcher() const;
    void abort() const;

    TCPConnData * detach() const;
    TCPManager * tcpManager() const;

    static LoginPass getBasicAuth(const ByteArray & authorization);

private:
    void callNextHandler() const;

private:
    class Shared;
    Shared * d;
    friend class impl::RequestParser;
};

class PassThroughHandler
{
public:
    virtual void morePassThroughData() = 0;
};

} // namespace
