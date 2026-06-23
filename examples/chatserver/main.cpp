/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <chatserver/services/chatservice.h>

#include <cflib/net/httpserver.h>
#include <cflib/net/rmiserver.h>
#include <cflib/net/wscommmanager.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>
#include <cflib/util/mainloop.h>
#include <cflib/util/unixsignal.h>

#include <iostream>

using namespace cflib::net;
using namespace cflib::util;
using namespace chatserver::services;

USE_LOG(LogCat::Network)

namespace {

auto && out = std::cout;
auto && err = std::cerr;

int showUsage(const ByteArray & executable)
{
    err
        << "Usage: " << executable.toStdString() << " [options]"          << std::endl
        << "Options:"                                                     << std::endl
        << "  -h, --help             => this help"                        << std::endl
        << "  -l, --log <level>      => set log level 1 -> all, 7 -> off" << std::endl;
    return 1;
}

}

int main(int argc, char * argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help         ('h', "help"             ); cmdLine << help;
    Option logOpt       ('l', "log",         true); cmdLine << logOpt;
    if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

    // start logging
    if (logOpt.isSet()) {
        Log::start("-");
        logInfo("chatserver started");
        Log::setLogLevel(logOpt.value().toUInt());
    }

    MainLoop loop;
    UnixSignal unixSignal(true);

    WSCommManager<int> commMgr("/ws");
    RMIServer<int>     rmiServer(commMgr);

    // services
    ChatService chatService; rmiServer.registerService(chatService);

    HttpServer serv(1);
    serv.registerHandler(commMgr);
    serv.start("0.0.0.0", 8000);

    out << "listening on port 8000" << std::endl;

    return loop.exec();
}
