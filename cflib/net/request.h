/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfcontainers.h>
#include <cflib/base/macros.h>

namespace cflib { namespace net {

class PassThroughHandler;
class RequestHandler;
class TCPConnData;
class TCPManager;
namespace impl { class RequestParser; }

class Request
{
public:
    typedef CFPair<int, int> Id;
    typedef CFMap<CFByteArray, CFByteArray> KeyVal;
    enum Method {
        NONE = 0,
        GET,
        POST,
        HEAD
    };
    struct LoginPass { CFString login; CFString password; };

public:
    Request();
    Request(int connId, int requestId,
        const CFByteArray & header,
        const KeyVal & headerFields, Method method, const CFByteArray & uri,
        const CFByteArray & body, const CFList<RequestHandler *> & handlers, bool passThrough,
        impl::RequestParser * parser);

    // implicit sharing
    ~Request();
    Request(const Request & other);
    Request & operator=(const Request & other);

    Id getId() const;
    bool replySent() const;

    CFByteArray getRawHeader() const;
    CFByteArray getHeader(const CFByteArray & name) const;
    CFByteArray getHostname() const;
    KeyVal getHeaderFields() const;
    Method getMethod() const;
    CFByteArray getMethodName() const;
    inline bool isGET()  const { return getMethod() == GET; }
    inline bool isPOST() const { return getMethod() == POST; }
    inline bool isHEAD() const { return getMethod() == HEAD; }
    CFByteArray getUri() const;
    CFByteArray getBody() const;
    CFByteArray getRemoteIP() const;
    LoginPass getBasicAuth() const;

    void sendNotFound() const;
    void sendRedirect(const CFByteArray & url) const;
    void sendReply(const CFByteArray & reply, const CFByteArray & contentType, bool compression = true) const;
    void sendText(const CFString & reply, const CFByteArray & contentType = "text/html", bool compression = true) const;
    void sendRaw(const CFByteArray & header, const CFByteArray & body, bool compression) const;
    void addHeaderLine(const CFByteArray & line) const;
    CFByteArray defaultHeaders() const;

    bool isPassThrough() const;
    void setPassThroughHandler(PassThroughHandler * hdl) const;
    CFByteArray readPassThrough(bool & isLast) const;
    void startWatcher() const;
    void abort() const;

    TCPConnData * detach() const;
    TCPManager * tcpManager() const;

    static LoginPass getBasicAuth(const CFByteArray & authorization);

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

}}    // namespace
