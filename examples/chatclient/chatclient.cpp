/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "chatclient.h"

#include <cflib/util/log.h>
#include <cflib/serialize/util.h>

#include <cstdio>
#include <unistd.h>

using namespace cflib::net;
using namespace cflib::serialize;
using namespace cflib::util;

USE_LOG(LogCat::Network)

ChatClient::ChatClient()
{
    client_.connected   .bind(this, &ChatClient::onConnected);
    client_.disconnected.bind(this, &ChatClient::onDisconnected);
    client_.receive     .bind(this, &ChatClient::onReceive);
}

ChatClient::~ChatClient()
{
    client_.shutdown();
}

void ChatClient::connect(const String & host, int port)
{
    logInfo("connecting to %1:%2", host.toUtf8().constData(), port);

    Url url("ws://" + host + ":" + String::number(port) + "/ws");
    client_.connect(url);
}

void ChatClient::disconnect()
{
    client_.disconnect();
}

void ChatClient::sendMessage(const String & message)
{
    if (!connected_) {
        logWarn("not connected yet");
        return;
    }

    BERSerializer ser(2);
    ser << String("chatservice") << String("void sendMessage(String)") << message;
    client_.send(ser.data(), true);
}

void ChatClient::onConnected()
{
    logInfo("connected to server");

    ByteArray ba;
    if (clientId_.isEmpty()) ba = emptyTag(1);
    else                     ba = toByteArray(clientId_, 1);
    client_.send(ba, true);

    connected_ = true;
    connected();
}

void ChatClient::onDisconnected()
{
    logInfo("disconnected from server");
    connected_ = false;
    clientId_.clear();
}

void ChatClient::onReceive(const ByteArray & data, bool isBinary)
{
    if (!isBinary) {
        logWarn("received non-binary data");
        return;
    }

    uint64 tag = 0;
    int tagLen = 0, lengthSize = 0;
    int32 valueLen = cflib::serialize::getTLVLength(data, tag, tagLen, lengthSize);

    if (tag == 1) {
        // Client ID
        if (valueLen == 20) {
            clientId_ = cflib::serialize::fromByteArray<ByteArray>(data, tagLen, lengthSize, (int32)valueLen);
            logInfo("got client ID (20 bytes)");
        } else {
            logWarn("wrong client ID length: %1", valueLen);
        }
    } else if (tag == 2) {
        // RMI response - just log for now
        BERDeserializer deser(data);
        String serviceName, signature;
        deser >> serviceName >> signature;
        logTrace("RMI response: %1::%2", serviceName.toUtf8().constData(), signature.toUtf8().constData());
    } else if (tag == 3) {
        // Signal from server (newMessage)
        BERDeserializer deser(data);
        uint regId;
        deser >> regId;
        // Read encoded params
        ByteArray encodedParams;
        deser >> encodedParams;

        BERDeserializer paramsDeser(encodedParams);
        String msg;
        paramsDeser >> msg;

        newMessage(msg);
    } else {
        logWarn("unknown tag: %1", tag);
    }
}
