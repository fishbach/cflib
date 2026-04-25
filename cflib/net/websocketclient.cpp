/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "websocketclient.h"

#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/util.h>
#include <cflib/net/impl/util.h>
#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Network)

namespace cflib::net {

namespace {

const uint retryDelayMs = 1000;

}

class WebSocketClient::Impl : public util::ThreadVerify
{
public:
    Impl(WebSocketClient & parent, const crypt::TLSCredentials & credentials) :
        ThreadVerify("WebSocketClient", Net),
        parent_(parent),
        mgr_(credentials.isEmpty() ? 0 : 1, this),
        reconnectTimer_(this, &Impl::doConnect),
        conn_(nullptr)
    {
        if (!credentials.isEmpty()) mgr_.clientCredentials() = credentials;
    }

    Impl(WebSocketClient & parent, const crypt::TLSCredentials & credentials, util::ThreadVerify * other) :
        ThreadVerify(other),
        parent_(parent),
        mgr_(credentials.isEmpty() ? 0 : 1, this),
        reconnectTimer_(this, &Impl::doConnect),
        conn_(nullptr)
    {
        if (!credentials.isEmpty()) mgr_.clientCredentials() = credentials;
    }

    ~Impl()
    {
        shutdown();
    }

    void shutdown()
    {
        mgr_.shutdown();
        stopVerifyThread();
    }

    void connect(const Url & url, const ByteArrayList & headers)
    {
        if (!verifyThreadCall(&Impl::connect, url, headers)) return;

        url_ = url;
        headers_ = headers;
        doConnect();
    }

    void disconnect();

    void send(const ByteArray & data, bool isBinary);

private:
    void doConnect();

private:
    WebSocketClient & parent_;
    TCPManager mgr_;
    util::EVTimer reconnectTimer_;

    Url url_;
    ByteArrayList headers_;

    class Conn;
    Conn * conn_;
    std::atomic<bool> connected_ = ATOMIC_VAR_INIT(false);
};

class WebSocketClient::Impl::Conn : public TCPConn, public util::ThreadVerify
{
public:
    Conn(WebSocketClient::Impl * impl, TCPConnData * data, const ByteArray & secWebsocketKey) :
        TCPConn(data),
        ThreadVerify(manager().networkThread()),
        impl_(impl),
        secWebsocketKey_(secWebsocketKey),
        deflateEnabled_(false),
        isBinary_(false),
        isDeflated_(false)
    {
        setNoDelay(true);
        startReadWatcher();
    }

    void send(const ByteArray & data, bool isBinary)
    {
        // do the compression first
        ByteArray payload;
        const int uncompressedPayloadSize = data.size();
        const bool deflate = deflateEnabled_ && uncompressedPayloadSize > 256;
        int payloadSize;
        if (deflate) {
            payload = data;
            util::deflateRaw(payload, 1);
            payloadSize = payload.size();
            logDebug("deflated %1 -> %2 (connId: %3)", uncompressedPayloadSize, payloadSize, id());
        } else {
            payloadSize = data.size();
        }

        // then calculate length
        // For details about the size of the length field, see impl::ws::writeLength
        int lengthSize = 10;
        if      (payloadSize < 126) lengthSize = 2;
        else if (payloadSize < 0x10000) lengthSize = 4;

        const int KeySize = 4;
        const int keyAndPayloadSize = KeySize + payloadSize;
        const int frameSize = lengthSize + keyAndPayloadSize;
        // frame: [<=10 byte length | 4 byte random key | payloadSize byte payload]

        // Fill frame
        ByteArray frame;
        frame.resize(frameSize);
        const uint8 * payloadData = !payload.isNull() ? (const uint8 *)payload.data() : (const uint8 *)data.data();
        memcpy(((uint8 *)frame.data()) + lengthSize + KeySize, payloadData, (size_t)payloadSize);

        impl::ws::maskPayload(frame, rng_.uint32(), (uint32)lengthSize);
        impl::ws::writeLength(frame,
            true, deflate, isBinary ? impl::ws::BinaryFrame : impl::ws::TextFrame,
            true, (uint64)keyAndPayloadSize, (uint64)lengthSize);

        write(frame);
    }

    void unlink()
    {
        impl_ = nullptr;
    }

    inline bool handshakeFinished() const
    {
        return secWebsocketKey_.isEmpty();
    }

protected:
    void newBytesAvailable() override
    {
        if (!verifyThreadCall(&Conn::newBytesAvailable)) return;

        buf_ += read();

        // handle HTTP header
        if (!handshakeFinished() && !headerOk()) return;

        forever {
            bool fin;
            bool rsv1;
            uint8 opcode;
            bool mask;         // Wether payload is masked
            uint64 len;       // Length of payload
            uint lengthEnd;    // Length of length field
            if (!impl::ws::readLength(buf_, fin, rsv1, opcode, mask, len, lengthEnd)) {
                startReadWatcher();
                return;
            }

            // server must not send masked data
            if (mask) {
                logWarn("mask in frame: %1", buf_);
                close(HardClosed, true);
                return;
            }

            const bool deflate = deflateEnabled_ && rsv1;
            ByteArray payload = buf_.mid(lengthEnd, len);
            buf_.remove(0, lengthEnd + len);

            // handle message types
            if (opcode == impl::ws::ContinuationFrame) {
                fragmentBuf_ += payload;
                if (fin) {
                    if (isDeflated_) util::inflateRaw(fragmentBuf_);
                    if (impl_) impl_->parent_.receive(fragmentBuf_, isBinary_);
                    fragmentBuf_.clear();
                }
            } else if (opcode == impl::ws::TextFrame || opcode == impl::ws::BinaryFrame) {
                if (!fin) {
                    isBinary_ = (opcode == impl::ws::BinaryFrame);
                    isDeflated_ = deflate;
                    fragmentBuf_ += payload;
                } else {
                    if (deflate) util::inflateRaw(payload);
                    if (impl_) impl_->parent_.receive(payload, opcode == impl::ws::BinaryFrame);
                }
            } else if (opcode == impl::ws::ConnectionClose) {
                logDebug("received close frame (connId: %1)", id());
                close(ReadWriteClosed, true);
                return;
            } else if (opcode == impl::ws::Ping) {
                // send pong
                impl::ws::maskPayload(payload, rng_.uint32());
                ByteArray pong;
                impl::ws::writeLength(pong,
                    true, deflate, impl::ws::Pong,
                    true, payload.length());
                pong += payload;
                write(pong);
            } else if (opcode == impl::ws::Pong) {
                logDebug("received pong (connId: %1)", id());
            } else  {
                logWarn("unknown opcode %1 in frame (%2)", opcode, buf_.toHex());
            }
        }
    }

    void closed(CloseType type) override
    {
        if (!verifyThreadCall(&Conn::closed, type)) return;
        logFunctionTraceParam("WebSocketClient::Impl::Conn::closed(%1) (connId: %2)", (int)type, id());

        if (type != HardClosed) {
            close(HardClosed, true);
            return;
        }

        if (impl_) {
            impl_->conn_ = nullptr;
            impl_->reconnectTimer_.singleShot(retryDelayMs / 1000.0);
            impl_->connected_ = false;
            impl_->parent_.disconnected();
        }

        util::deleteNext(this);
    }

private:
    bool headerOk()
    {
        const int headerEndPos = buf_.indexOf("\r\n\r\n");
        if (headerEndPos == -1) {
            startReadWatcher();
            return false;
        }

        int status;
        ByteArray statusText;
        MultiMap<ByteArray, ByteArray> fields;
        if (!impl::parseResponseHeader(buf_.left(headerEndPos), status, statusText, fields)) {
            logWarn("broken header in reply (connId: %1)", id());
            close(HardClosed, true);
            return false;
        }
        buf_.remove(0, headerEndPos + 4);

        if (status != 101) {
            logWarn("wrong HTTP status in reply (connId: %1): %2 %3", id(), status, statusText);
            close(HardClosed, true);
            return false;
        }

        if (fields.value("upgrade").toLower() != "websocket") {
            logWarn("no websocket upgrade in reply (connId: %1): %2", id(), buf_);
            close(HardClosed, true);
            return false;
        }

        const ByteArray secWebSocketAccept = fields.value("sec-websocket-accept");
        if (secWebSocketAccept.isNull()) {
            logWarn("no websocket accept in reply (connId: %1): %2", id(), buf_);
            close(HardClosed, true);
            return false;
        }

        if (secWebSocketAccept != crypt::sha1(secWebsocketKey_ + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toBase64()) {
            logWarn("websocket accept key does not match (connId: %1): %2", id(), buf_);
            close(HardClosed, true);
            return false;
        }

        const ByteArray secWebSocketExtensions = fields.value("sec-websocket-extensions");
        if (secWebSocketExtensions.contains("permessage-deflate")) {
            logDebug("using permessage-deflate (connId: %1)", id());
            deflateEnabled_ = true;
        }

        secWebsocketKey_.clear();

        // allow connect handler to send some initial data
        if (impl_) {
            impl_->connected_ = true;
            impl_->parent_.connected();
        }

        return true;
    }

private:
    WebSocketClient::Impl * impl_;
    ByteArray secWebsocketKey_;
    ByteArray buf_;
    bool deflateEnabled_;

    ByteArray fragmentBuf_;
    bool isBinary_;
    bool isDeflated_;

    crypt::FastRandom rng_;
};

void WebSocketClient::Impl::disconnect()
{
    if (!verifySyncedThreadCall(&Impl::disconnect)) return;

    reconnectTimer_.stop();

    if (conn_) {
        conn_->unlink();
        conn_->close(TCPConn::HardClosed, true);
        conn_ = nullptr;
    }

    connected_ = false;
    parent_.disconnected();
}

void WebSocketClient::Impl::send(const ByteArray & data, bool isBinary)
{
    if (!verifyThreadCall(&Impl::send, data, isBinary)) return;

    if (conn_ && conn_->handshakeFinished()) conn_->send(data, isBinary);
}

void WebSocketClient::Impl::doConnect()
{
    if (conn_) disconnect();

    TCPConnData * cd = nullptr;
    if (mgr_.clientCredentials().isEmpty()) {
        cd = mgr_.openConnection(
            url_.host().toUtf8(),
            url_.port() != -1 ? url_.port() : 80);
    } else {
        cd = mgr_.openConnection(
            url_.host().toUtf8(),
            url_.port() != -1 ? url_.port() : 443);
    }
    if (!cd) {
        reconnectTimer_.singleShot(retryDelayMs / 1000.0);
        return;
    }

    const ByteArray secWebsocketKey = crypt::random(16).toBase64();

    conn_ = new Conn(this, cd, secWebsocketKey);

    ByteArrayList headers = headers_;
    headers
        << "Upgrade: websocket"
        << "Connection: Upgrade"
        << "Sec-WebSocket-Key: " << secWebsocketKey
        << "Origin: " << url_.scheme().toUtf8() << "://" << url_.host().toUtf8()
        << (url_.port() != -1 ? ":" + ByteArray::number(url_.port()) : "")
        << "Sec-WebSocket-Version: 13"
        << "Sec-WebSocket-Extensions: permessage-deflate";

    conn_->write(impl::createHttpRequest("GET", url_, {}, headers, false));
}

// ============================================================================

WebSocketClient::WebSocketClient(const crypt::TLSCredentials & credentials, util::ThreadVerify * other) :
    impl_(other ?
        new Impl(*this, credentials, other) :
        new Impl(*this, credentials))
{
}

WebSocketClient::~WebSocketClient()
{
    delete impl_;
}

void WebSocketClient::shutdown()
{
    impl_->shutdown();
}

void WebSocketClient::connect(const Url & url, const ByteArrayList & headers)
{
    impl_->connect(url, headers);
}

void WebSocketClient::disconnect()
{
    impl_->disconnect();
}

void WebSocketClient::send(const ByteArray & data, bool isBinary)
{
    impl_->send(data, isBinary);
}

} // namespace
