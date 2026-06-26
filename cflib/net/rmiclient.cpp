/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rmiclient.h"

#include <cflib/net/rsigclient.h>
#include <cflib/net/websocketclient.h>
#include <cflib/serialize/util.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>

USE_LOG(LogCat::Network)

namespace cflib::net {

class RMIClient::Impl : public util::ThreadVerify
{
    CF_DISABLE_COPY(Impl)
public:
    Impl(RMIClient & parent, const crypt::TLSCredentials & credentials) :
        ThreadVerify("RMIClient", Net),
        parent_(parent),
        ws_(credentials, this),
        aliveTimeoutTimer_(this, &Impl::checkAliveTimeout)
    {
        init();
    }

    Impl(RMIClient & parent, const crypt::TLSCredentials & credentials, util::ThreadVerify * other) :
        ThreadVerify(other),
        parent_(parent),
        ws_(credentials, this),
        aliveTimeoutTimer_(this, &Impl::checkAliveTimeout)
    {
        init();
    }

    ~Impl()
    {
        shutdown();
    }

    void init()
    {
        ws_.connected.bind(this, &Impl::wsConnected);
        ws_.disconnected.bind(this, &Impl::wsDisconnected);
        ws_.receive.bind(this, &Impl::wsReceive);
    }

    void shutdown()
    {
        ws_.shutdown();
        stopVerifyThread();
    }

    void connect(const Url & url, const ByteArrayList & headers)
    {
        if (!verifyThreadCall(&Impl::connect, url, headers)) return;

        url_ = url;
        headers_ = headers;
        doConnect();
    }

    void disconnect()
    {
        if (!verifySyncedThreadCall(&Impl::disconnect)) return;

        ws_.disconnect();
    }

    void sendRequest(const ByteArray & data, const std::function<void (const ByteArray &)> & callback)
    {
        if (!verifyThreadCall(&Impl::sendRequest, data, callback)) return;

        if (requestActive_) {
            waitingRequests_ << Request{data, callback};
        } else {
            requestActive_ = true;
            requestCallback_ = callback;
            ws_.send(data, true);
        }
    }

    void sendAsync(const ByteArray & data, bool doNotBuffer)
    {
        if (!verifyThreadCall(&Impl::sendAsync, data, doNotBuffer)) return;

        ws_.send(data, true);
        CF_UNUSED(doNotBuffer);
    }

    size_t nextRSigId() { return ++nextRsigId_; }

    void registerRSig(RSigClientBase * rsig, uint64 id, const ByteArray & regData)
    {
        if (!verifyThreadCall(&Impl::registerRSig, rsig, id, regData)) return;

        rsigHandlers_[id] = rsig;
        ws_.send(regData, true);
    }

    void unregisterRSig(uint64 rsigId, const ByteArray & unregData)
    {
        if (!verifySyncedThreadCall(&Impl::unregisterRSig, rsigId, unregData)) return;

        RSigClientBase * rsig = rsigHandlers_.value(rsigId);
        if (!rsig) return;
        rsigHandlers_.erase(rsigId);
        ws_.send(unregData, true);
     }

    void setAliveTimeoutHandler(uint timeoutMs, const std::function<void (bool timeout)> & func)
    {
        if (!verifyThreadCall(&Impl::setAliveTimeoutHandler, timeoutMs, func)) return;

        lastAlive_ = DateTime::currentDateTimeUtc();
        timeoutMs_ = timeoutMs;
        timeoutFunc_ = func;
        aliveTimeoutTimer_.start(timeoutMs / 2000.0);
    }

    void registerHandler(uint tagNo, const std::function<void (const ByteArray &)> & func)
    {
        if (!verifyThreadCall(&Impl::registerHandler, tagNo, func)) return;

        msgHandlers_[tagNo] = func;
    }

private:
    void doConnect()
    {
        serialize::BERSerializer ser(1);
        if (!clientId_.isNull()) {
            ser << clientId_;
        }
        ws_.connect(url_, headers_);
    }

    void wsConnected()
    {
        logDebug("WebSocket connected");

        serialize::BERSerializer ser(1);
        if (!clientId_.isNull()) {
            ser << clientId_;
        }
        ws_.send(ser.data(), true);

        // for (auto & [id, data] : rsigHandlers_) {
        //     sendRsigRegistration(data.service, data.name, true, id);
        // }

        for (auto & data : waitingAsync_) {
            ws_.send(data, true);
        }
        waitingAsync_.clear();

        checkWaitingRequests();
    }

    void wsDisconnected()
    {
        logDebug("WebSocket disconnected");

        connected_ = false;
        requestActive_ = false;
        requestCallback_ = nullptr;
        parent_.disconnected();
    }

    void wsReceive(const ByteArray & data, bool isBinary)
    {
        if (!isBinary) {
            logDebug("received text message: %1", String::fromUtf8(data));
            return;
        }

        uint64 tag = 0;
        int tagLen = 0;
        int lengthSize = 0;
        int32 valueLen = serialize::getTLVLength(data, tag, tagLen, lengthSize);
        if (valueLen < 0) {
            logWarn("broken BER msg");
            return;
        }

        const uint8 * valuePtr = (const uint8 *)data.constData() + tagLen + lengthSize;

        switch (tag) {
            case 1: {
                clientId_ = serialize::fromByteArray<ByteArray>(data, tagLen, lengthSize, valueLen);
                logDebug("received clientId: %1", clientId_.toHex());
                parent_.connected();
                break;
            }
            case 2:
                if (requestCallback_) {
                    requestCallback_(ByteArray((const char *)valuePtr, valueLen));
                    checkWaitingRequests();
                }
                break;
            case 3: {
                uint64 rsigId;
                ByteArray paramsData;
                serialize::BERDeserializer(data, valuePtr, valueLen) >> rsigId >> paramsData;
                logDebug("RSig %1 received", rsigId);
                RSigClientBase * rsig = rsigHandlers_.value(rsigId);
                if (rsig) rsig->call(paramsData);
                break;
            }
            case 4: {
                ws_.send(data, true);
                break;
            }
            case 5:
                lastAlive_ = DateTime::currentDateTimeUtc();
                if (aliveTimeout_) {
                    aliveTimeout_ = false;
                    if (timeoutFunc_) timeoutFunc_(false);
                }
                break;
            default: {
                auto it = msgHandlers_.find(tag);
                if (it != msgHandlers_.end()) {
                    it->second(ByteArray((const char *)valuePtr, valueLen));
                }
                break;
            }
        }
    }

    void checkWaitingRequests()
    {
        if (!waitingRequests_.isEmpty()) {
            auto req = waitingRequests_.takeFirst();
            requestCallback_ = req.callback;
            ws_.send(req.data, true);
        } else {
            requestActive_ = false;
            requestCallback_ = nullptr;
        }
    }

    void checkAliveTimeout()
    {
        if (!verifyThreadCall(&Impl::checkAliveTimeout)) return;

        DateTime now = DateTime::currentDateTimeUtc();
        if (now.secsTo(lastAlive_) > (int64)timeoutMs_ && !aliveTimeout_) {
            aliveTimeout_ = true;
            if (timeoutFunc_) timeoutFunc_(true);
        }
    }

private:
    struct Request {
        ByteArray data;
        std::function<void (const ByteArray &)> callback;
    };

    RMIClient & parent_;
    WebSocketClient ws_;
    util::EVTimer aliveTimeoutTimer_;

    Url url_;
    ByteArrayList headers_;
    ByteArray clientId_;

    bool connected_ = false;
    bool requestActive_ = false;
    std::function<void (const ByteArray &)> requestCallback_;
    List<Request> waitingRequests_;
    List<ByteArray> waitingAsync_;

    Hash<uint64, RSigClientBase *> rsigHandlers_;
    AtomicUInt nextRsigId_;

    Map<uint64, std::function<void (const ByteArray &)>> msgHandlers_;

    DateTime lastAlive_;
    uint timeoutMs_ = 0;
    std::function<void (bool timeout)> timeoutFunc_;
    bool aliveTimeout_ = false;
};

RMIClient::RMIClient(const crypt::TLSCredentials & credentials, util::ThreadVerify * other) :
    impl_(other ? new Impl(*this, credentials, other) : new Impl(*this, credentials))
{
}

RMIClient::~RMIClient()
{
    delete impl_;
}

void RMIClient::shutdown()
{
    impl_->shutdown();
}

void RMIClient::connect(const Url & url, const ByteArrayList & headers)
{
    impl_->connect(url, headers);
}

void RMIClient::disconnect()
{
    impl_->disconnect();
}

void RMIClient::sendRequest(const ByteArray & data, const std::function<void (const ByteArray &)> & callback)
{
    impl_->sendRequest(data, callback);
}

void RMIClient::sendAsync(const ByteArray & data, bool doNotBuffer)
{
    impl_->sendAsync(data, doNotBuffer);
}

size_t RMIClient::nextRSigId()
{
    return impl_->nextRSigId();
}

void RMIClient::registerRSig(RSigClientBase * rsig, uint64 id, const ByteArray & regData)
{
    impl_->registerRSig(rsig, id, regData);
}

void RMIClient::unregisterRSig(uint64 rsigId, const ByteArray & unregData)
{
    impl_->unregisterRSig(rsigId, unregData);
}

void RMIClient::setAliveTimeoutHandler(uint timeoutMs, const std::function<void (bool timeout)> & func)
{
    impl_->setAliveTimeoutHandler(timeoutMs, func);
}

void RMIClient::registerHandler(uint tagNo, const std::function<void (const ByteArray &)> & func)
{
    impl_->registerHandler(tagNo, func);
}

} // namespace cflib::net
