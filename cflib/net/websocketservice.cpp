/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "websocketservice.h"

#include <cflib/crypt/util.h>
#include <cflib/net/tcpconn.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

// Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits
// Sec-WebSocket-Extensions: permessage-deflate
// Sec-WebSocket-Extensions: x-webkit-deflate-frame

namespace cflib { namespace net {

class WebSocketService::WSConnHandler : public util::ThreadVerify, public TCPConn
{
public:
    WSConnHandler(WebSocketService * service, TCPConnData * connData, uint connId, uint connectionTimeoutSec,
        bool deflate, const Request::KeyVal & savedHeaders)
    :
        ThreadVerify(service),
        TCPConn(connData, 0x10000, connectionTimeoutSec > 0),
        savedHeaders(savedHeaders),
        service_(*service),
        connId_(connId),
        connectionSendInterval_(connectionTimeoutSec / 2),
        connectionDataTimeout_(connectionTimeoutSec * 3 / 2),
        isBinary_(false),
        isDeflated_(false),
        ping_("\x89\x00", 2),
        deflateEnabled_(deflate)
    {
        logTrace("WSConnHandler(%1)", connId_);
        setNoDelay(true);
        startReadWatcher();
        if (connectionTimeoutSec > 0) {
            lastRead_  = CFDateTime::currentDateTimeUtc();
            lastWrite_ = CFDateTime::currentDateTimeUtc();
        }
        if (deflateEnabled_) logDebug("using deflate on connection: %1", connId);
    }

    ~WSConnHandler()
    {
        logTrace("~WSConnHandler(%1)", connId_);
    }

    void continueRead()
    {
        while (buf_.size() >= 2) {
            uint rv = handleData();
            if (rv == 0) return;
            if (rv == 2) break;
        }
        startReadWatcher();
    }

    void send(const ByteArray & data, bool isBinary)
    {
        if (logTrace) {
            if (isBinary) logTrace("binary out %1: %2", connId_, data.toHex());
            else          logTrace("text out %1: %2",   connId_, data);
        }

        bool deflate = false;
        ByteArray deflateBuf;
        if (deflateEnabled_ && data.size() > 256) {
            deflate = true;
            deflateBuf = data;
            util::deflateRaw(deflateBuf, 1);
            logDebug("deflated %1 -> %2 (connId: %3)", (cfuint64)data.size(), (cfuint64)deflateBuf.size(), connId_);
        }

        const uint len = deflate ? deflateBuf.size() : data.size();
        ByteArray frame;
        frame.reserve(len + 10);

        // write first start byte
        frame += isBinary ? (deflate ? 0xC2 : 0x82) : (deflate ? 0xC1 : 0x81);

        // write length
        if (len < 126) {
            frame += len;
        } else if (len < 0x10000) {
            frame += 126;
            frame += len >> 8;
            frame += len & 0xFF;
        } else {
            frame += 127;
            frame += (char)0;    // (len >> 56) & 0xFF;
            frame += (char)0;    // (len >> 48) & 0xFF;
            frame += (char)0;    // (len >> 40) & 0xFF;
            frame += (char)0;    // (len >> 32) & 0xFF;
            frame += (len >> 24) & 0xFF;
            frame += (len >> 16) & 0xFF;
            frame += (len >>  8) & 0xFF;
            frame += len & 0xFF;
        }

        frame += deflate ? deflateBuf : data;
        write(frame);
    }

    void checkTimeout(const CFDateTime & now)
    {
        cfint64 readSecs = lastRead_.secsTo(now);
        cfint64 writeSecs = lastWrite_.secsTo(now);
        uint last = (uint)cfMax(readSecs, writeSecs);
        if (last < connectionSendInterval_) return;
        if (last > connectionDataTimeout_) {
            logInfo("timeout on connection %1", connId_);
            close(HardClosed, true);
        } else {
            write(ping_);
        }
    }

protected:
    virtual void newBytesAvailable()
    {
        if (!verifyThreadCall(&WSConnHandler::newBytesAvailable)) return;

        buf_ += read();
        if (connectionDataTimeout_ > 0) lastRead_ = CFDateTime::currentDateTimeUtc();
        continueRead();
    }

    virtual void closed(CloseType type)
    {
        if (!verifyThreadCall(&WSConnHandler::closed, type)) return;

        if ((type & ReadClosed) && (type & WriteClosed)) {
            service_.connections_.erase(connId_);
            util::deleteNext(this);
        }
        service_.closed(connId_, type);
    }

    virtual void someBytesWritten(cfuint64 count)
    {
        if (!verifyThreadCall(&WSConnHandler::someBytesWritten, count)) return;
        lastWrite_ = CFDateTime::currentDateTimeUtc();
    }

private:
    // 0 -> stop, 1 -> continue, 2 -> need more data
    uint handleData()
    {
        cfuint8 * data = (cfuint8 *)buf_.constData();
        uint dLen = buf_.size();
        const bool fin = data[0] & 0x80;
        const bool deflate = deflateEnabled_ && (data[0] & 0x40);
        const cfuint8 opcode = data[0] & 0xF;
        const bool mask = data[1] & 0x80;

        // clients must send masked data
        if (!mask) {
            logWarn("no mask in frame: %1", buf_);
            close(HardClosed, true);
            return 0;
        }

        // read len
        cfuint64 len = data[1] & 0x7F;
        if (len < 126) {
            data += 2;
            dLen -= 2;
        } else if (len == 126) {
            if (dLen < 4) return 2;
            len = (cfuint64)data[2] << 8 | (cfuint64)data[3];
            data += 4;
            dLen -= 4;
        } else {
            if (dLen < 10) return 2;
            len =
                (cfuint64)data[2] << 56 | (cfuint64)data[3] << 48 | (cfuint64)data[4] << 40 | (cfuint64)data[5] << 32 |
                (cfuint64)data[6] << 24 | (cfuint64)data[7] << 16 | (cfuint64)data[8] <<  8 | (cfuint64)data[9];
            data += 10;
            dLen -= 10;
        }

        // Enough data available?
        if (dLen < len + 4) return 2;

        // apply mask
        const cfuint8 * maskKey = data;
        data += 4;
        dLen -= 4;
        for (uint i = 0 ; i < len ; ++i) data[i] ^= maskKey[i % 4];

        bool stopRead = false;

        // handle message types
        if (opcode == 0x0) {    // continuation frame
            fragmentBuf_.append((const char *)data, len);
            if (fin) {
                if (isDeflated_) util::inflateRaw(fragmentBuf_);
                if (logTrace) {
                    if (isBinary_) logTrace("binary in %1: %2", connId_, fragmentBuf_.toHex());
                    else           logTrace("text in %1: %2",   connId_, fragmentBuf_);
                }
                service_.newMsg(connId_, fragmentBuf_, isBinary_, stopRead);
                fragmentBuf_.clear();
            }
        } else if (opcode == 0x1 || opcode == 0x2) {    // test / binary frame
            if (!fin) {
                isBinary_ = opcode == 2;
                isDeflated_ = deflate;
                fragmentBuf_.append((const char *)data, len);
            } else {
                ByteArray msg((const char *)data, len);
                if (deflate) util::inflateRaw(msg);
                if (logTrace) {
                    if (opcode == 2) logTrace("binary in %1: %2", connId_, msg.toHex());
                    else             logTrace("text in %1: %2",   connId_, msg);
                }
                service_.newMsg(connId_, msg, opcode == 2, stopRead);
            }
        } else if (opcode == 0x8) {    // connection close
            logDebug("received close frame");
            close(ReadWriteClosed, true);
            stopRead = true;
        } else if (opcode == 0x9) {    // ping
            // send pong
            cfuint8 * orig = (cfuint8 *)buf_.constData();
            orig[0] = (orig[0] & 0xF0) | 0xA;
            orig[1] &= 0x7F;
            ByteArray pong((const char *)orig, buf_.size() - dLen - 4);
            pong.append((const char *)data, len);
            write(pong);
        } else if (opcode == 0xA) {
            // pong
        } else  {
            logWarn("unknown opcode %1 in frame (%2)", opcode, buf_.toHex());
        }

        buf_.remove(0, buf_.size() - dLen + len);
        return stopRead ? 0 : 1;
    }

public:
    const Request::KeyVal savedHeaders;

private:
    WebSocketService & service_;
    const uint connId_;
    const uint connectionSendInterval_;
    const uint connectionDataTimeout_;
    ByteArray buf_;
    ByteArray fragmentBuf_;
    bool isBinary_;
    bool isDeflated_;
    CFDateTime lastRead_;
    CFDateTime lastWrite_;
    const ByteArray ping_;
    const bool deflateEnabled_;
};

// ============================================================================

WebSocketService::WebSocketService(const String & path, const CFRegex & allowedOrigin,
    uint connectionTimeoutSec)
:
    ThreadVerify("WebSocketService", LoopType::Worker),
    path_(path),
    allowedOrigin_(allowedOrigin),
    connectionTimeoutSec_(connectionTimeoutSec),
    lastConnId_(0),
    timer_(connectionTimeoutSec > 0 ? new util::EVTimer(this, &WebSocketService::checkTimeout) : 0)
{
    setThreadPrio(0); // should be HighPriority
    if (timer_) startTimer();
}

WebSocketService::~WebSocketService()
{
    delete timer_;
}

void WebSocketService::saveHeaderField(const ByteArray & field)
{
    saveHeaderFields_ << field;
}

void WebSocketService::send(uint connId, const ByteArray & data, bool isBinary)
{
    WSConnHandler * wsHdl = cfHashValue(connections_, connId, (WSConnHandler *)nullptr);
    if (wsHdl) wsHdl->send(data, isBinary);
}

void WebSocketService::close(uint connId, TCPConn::CloseType type)
{
    WSConnHandler * wsHdl = cfHashValue(connections_, connId, (WSConnHandler *)nullptr);
    if (wsHdl) wsHdl->close(type, true);
}

ByteArray WebSocketService::getRemoteIP(uint connId) const
{
    WSConnHandler * wsHdl = cfHashValue(connections_, connId, (WSConnHandler *)nullptr);
    if (wsHdl) return wsHdl->peerIP();
    return ByteArray();
}

ByteArray WebSocketService::getHeader(uint connId, const ByteArray & header) const
{
    WSConnHandler * wsHdl = cfHashValue(connections_, connId, (WSConnHandler *)nullptr);
    if (wsHdl) return cfMapValue(wsHdl->savedHeaders, header);
    return ByteArray();
}

void WebSocketService::continueRead(uint connId)
{
    WSConnHandler * wsHdl = cfHashValue(connections_, connId, (WSConnHandler *)nullptr);
    if (wsHdl) wsHdl->continueRead();
}

void WebSocketService::newConnection(uint)
{
}

void WebSocketService::closed(uint, TCPConn::CloseType)
{
}

void WebSocketService::handleRequest(const Request & request)
{
    if (request.getUri() != path_.toUtf8() || !request.isGET()) return;

    // check WS headers
    const Request::KeyVal headers = request.getHeaderFields();
    const ByteArray wsKey = cfMapValue(headers, ByteArray("sec-websocket-key"));
    if (cfMapValue(headers, ByteArray("upgrade")).toLower() != "websocket" || wsKey.isEmpty()) {
        request.sendNotFound();
        return;
    }
    const bool deflate = cfMapValue(headers, ByteArray("sec-websocket-extensions")).toLower().indexOf("permessage-deflate") != -1;

    // check origin
    if (allowedOrigin_.isValid() && !allowedOrigin_.match(cfMapValue(headers, ByteArray("origin")).toLower())) {
        logWarn("wrong Origin: %1", cfMapValue(headers, ByteArray("origin")));
        request.sendNotFound();
        return;
    }

    Request::KeyVal savedHeaders;
    for (const ByteArray & header : saveHeaderFields_) {
        auto it = headers.find(header);
        if (it != headers.end()) savedHeaders[header] = it->second;
    }

    // detach from socket
    TCPConnData * connData = request.detach();
    if (!connData) {
        logWarn("could not detach from socket");
        return;
    }

    addConnection(connData, wsKey, deflate, savedHeaders);
}

void WebSocketService::addConnection(TCPConnData * connData, const ByteArray & wsKey, bool deflate,
    const Request::KeyVal & savedHeaders)
{
    if (!verifyThreadCall(&WebSocketService::addConnection, connData, wsKey, deflate, savedHeaders)) return;

    logFunctionTrace

    const uint connId = ++lastConnId_;
    WSConnHandler * wsHdl = new WSConnHandler(this, connData, connId, connectionTimeoutSec_, deflate, savedHeaders);
    connections_[connId] = wsHdl;

    // write WS header
    ByteArray header =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: ";
    header += cflib::crypt::sha1(wsKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toBase64();
    header += "\r\n";
    // firefox bug: it does not accept "server_no_context_takeover"
    if (deflate) header += "Sec-WebSocket-Extensions: permessage-deflate; client_no_context_takeover\r\n";
    header += "\r\n";
    wsHdl->write(header);

    newConnection(connId);
}

void WebSocketService::startTimer()
{
    if (!verifyThreadCall(&WebSocketService::startTimer)) return;
    timer_->start(connectionTimeoutSec_ / 4.0);
}

void WebSocketService::checkTimeout()
{
    const CFDateTime now = CFDateTime::currentDateTimeUtc();
    for (auto & [id, hdl] : connections_) hdl->checkTimeout(now);
}

}}    // namespace
