/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <remote/chatserver/services/chatservice.h>

#include <cflib/net/rmiclient.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>

#include <iostream>

using namespace cflib::net;
using namespace cflib::util;
using namespace chatserver::dao;
using namespace remote::chatserver::services;

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

    Semaphore sem;
    rmiClient.connected.bind([&]() {
        out << "connected!" << std::endl;
        sem.release();
    });
    out << "connecting ..." << std::endl;
    logInfo("connecting ...");
    rmiClient.connect(url);
    sem.acquire();
    out << "go ..." << std::endl;

    ChatService chatService(rmiClient);
    chatService.newMessage.reg().bind([](const Message & msg) {
        out << "Message: " << msg.text.toUtf8().constCharPtr() << std::endl;
    });

    out << "testing ..." << std::endl;
    if (chatService.test()) out << "test ok" << std::endl;
    else                    out << "test failed" << std::endl;

    out << "Type messages to send (Ctrl-D to exit)" << std::endl;
    std::string line;
    while (std::getline(std::cin, line)) {
        chatService.sendMessage(String(line.c_str()));
    }

    out << "finished" << std::endl;
    logInfo("stopping ...");
    return 0;
}
