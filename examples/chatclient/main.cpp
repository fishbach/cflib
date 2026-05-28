/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <chatserver/services/chatservice.h>

#include <cflib/net/rmiclient.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>

#include <iostream>

using namespace cflib::net;
using namespace cflib::util;
using namespace chatserver::dao;
using namespace chatserver::services;

USE_LOG(LogCat::Network)

namespace {

auto && out = std::cout;
auto && err = std::cerr;

}

int main(int argc, char *argv[])
{
    CmdLine cmdLine(argc, argv);
    Arg hostArg(false); cmdLine << hostArg;
    Arg portArg(false); cmdLine << portArg;
    if (!cmdLine.parse()) {
        err << "Usage: chatclient <ip> <port>\n";
        return 1;
    }

    Log::start("chatclient.log");
    Log::setLogLevel(1);
    logInfo("chatclient started");

    RMIClient rmiClient;
    Url url("ws://" + hostArg.value() + ":" + portArg.value() + "/ws");

    rmiClient.connected.bind([]() {
        out << "connected, type messages to send (Ctrl-D to exit)" << std::endl;
    });
    out << "connecting ..." << std::endl;
    logInfo("connecting ...");
    rmiClient.connect(url);

    ChatService chatService(rmiClient);
    chatService.newMessage.reg().bind([](const Message & msg) {
        out << "Message: " << msg.text.toUtf8().constData() << std::endl;
    });

    if (chatService.test()) out << "test ok" << std::endl;
    else                    out << "test failed" << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        chatService.sendMessage(String(line.c_str()));
    }

    logInfo("stopping ...");
    return 0;
}
