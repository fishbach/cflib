/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/net/request.h>
#include <cflib/net/requesthandler.h>
#include <cflib/net/tcpconn.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace util { class EVTimer; }}

namespace cflib { namespace net {

class WebSocketService : public RequestHandler, public util::ThreadVerify
{
public:
    WebSocketService(const CFString & path, const CFRegex & allowedOrigin = CFRegex(),
        uint connectionTimeoutSec = 0);
    ~WebSocketService();

protected:
    void saveHeaderField(const CFByteArray & field);

    void send(uint connId, const CFByteArray & data, bool isBinary);
    void close(uint connId, TCPConn::CloseType type = TCPConn::ReadWriteClosed);
    void continueRead(uint connId);

    CFByteArray getRemoteIP(uint connId) const;
    CFByteArray getHeader(uint connId, const CFByteArray & header) const;

    virtual void newConnection(uint connId);
    virtual void newMsg(uint connId, const CFByteArray & data, bool isBinary, bool & stopRead) = 0;
    virtual void closed(uint connId, TCPConn::CloseType type);

    virtual void handleRequest(const Request & request);

private:
    void addConnection(TCPConnData * connData, const CFByteArray & wsKey, bool deflate,
        const Request::KeyVal & savedHeaders);
    void startTimer();
    void checkTimeout();

private:
    const CFString path_;
    const CFRegex allowedOrigin_;
    const uint connectionTimeoutSec_;
    CFSet<CFByteArray> saveHeaderFields_;
    class WSConnHandler;
    CFHash<uint, WSConnHandler *> connections_;
    uint lastConnId_;
    util::EVTimer * timer_;
};

}}    // namespace
