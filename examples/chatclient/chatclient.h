/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/websocketclient.h>
#include <cflib/util/sig.h>

class ChatClient
{
public:
    ChatClient();
    ~ChatClient();

    void connect(const String & host, int port);
    void disconnect();

    void sendMessage(const String & message);

cfsignals:
    sig<void (const String & msg)> newMessage;

private:
    void onConnected();
    void onDisconnected();
    void onReceive(const ByteArray & data, bool isBinary);

private:
    cflib::net::WebSocketClient client_;
    ByteArray clientId_;
    bool connected_ = false;
};
