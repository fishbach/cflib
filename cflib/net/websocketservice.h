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

namespace cflib::util { class EVTimer; }

namespace cflib::net {

class WebSocketService : public RequestHandler, public util::ThreadVerify
{
public:
    WebSocketService(const String & path, const CFRegex & allowedOrigin = CFRegex(),
        uint connectionTimeoutSec = 0);
    ~WebSocketService();

protected:
    void saveHeaderField(const ByteArray & field);

    void send(uint connId, const ByteArray & data, bool isBinary);
    void close(uint connId, TCPConn::CloseType type = TCPConn::ReadWriteClosed);
    void continueRead(uint connId);

    ByteArray getRemoteIP(uint connId) const;
    ByteArray getHeader(uint connId, const ByteArray & header) const;

    virtual void newConnection(uint connId);
    virtual void newMsg(uint connId, const ByteArray & data, bool isBinary, bool & stopRead) = 0;
    virtual void closed(uint connId, TCPConn::CloseType type);

    virtual void handleRequest(const Request & request);

private:
    void addConnection(TCPConnData * connData, const ByteArray & wsKey, bool deflate,
        const Request::KeyVal & savedHeaders);
    void startTimer();
    void checkTimeout();

private:
    const String path_;
    const CFRegex allowedOrigin_;
    const uint connectionTimeoutSec_;
    Set<ByteArray> saveHeaderFields_;
    class WSConnHandler;
    Hash<uint, WSConnHandler *> connections_;
    uint lastConnId_;
    util::EVTimer * timer_;
};

} // namespace
