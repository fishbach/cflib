/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <infoservice.h>

#include <cflib/net/fileserver.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/logservice.h>
#include <cflib/net/requestlog.h>
#include <cflib/net/rmiserver.h>
#include <cflib/net/wscommmanager.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>

#include <csignal>
#include <cstdio>
#include <unistd.h>

using namespace cflib::net;
using namespace cflib::util;

USE_LOG(LogCat::Etc)

int showUsage(const CFByteArray & executable)
{
    fprintf(stderr,
        "\nSimple Webchat example\n"
        "------------------------------------\n"
        "\nTo make this example run start the webchat executable and access the url\n"
        "http://127.0.0.1:8080/ through your browser.\n"
        "If you have an out of source build you need to redirect the webchat to\n"
        "the correct location of the example htdocs directory through:\n"
        "./%s --htdocs <path to cflib source>/examples/webchat/htdocs\n"
        "\nUsage: %s [options]\n"
        "Options:\n"
        "  -h, --help     => this help\n"
        "  -e, --export   => export Classes as Javascrpt classes\n"
        "  -f, --htdocs   => set path to htdocs (default htdocs in current directory)\n\n",
        executable.constData(), executable.constData());
    return 1;
}

int main(int argc, char *argv[])
{
    // parse cmd line
    CmdLine cmdLine(argc, argv);
    Option helpOpt      ('h', "help",  false); cmdLine << helpOpt;
    Option exportOpt    ('e', "export", true); cmdLine << exportOpt;
    Option htdocsPathOpt('f', "htdocs", true); cmdLine << htdocsPathOpt;
    if (!cmdLine.parse() || helpOpt.isSet()) return showUsage(cmdLine.executable());

    Log::start("webchat.log");
    logInfo("webchat started");

    RequestLog requestLog;

    FileServer fs(htdocsPathOpt.value("htdocs"));

    WSCommManager<CFString> commMgr("/ws");
    RMIServer<CFString> rmiServer(commMgr);
    InfoService infoService; rmiServer.registerService(infoService);
    LogService  logService;  rmiServer.registerService(logService);

    if (exportOpt.isSet()) {
        CFString dest(exportOpt.value());
        logInfo("exporting to: %1", dest.c_str());
        rmiServer.exportTo(dest);
        fs.exportTo(dest);
        return 0;
    }

    HttpServer serv;
    serv.registerHandler(requestLog);
    serv.registerHandler(commMgr);
    serv.registerHandler(rmiServer);
    serv.registerHandler(fs);
    serv.start("127.0.0.1", 8080);

    // Block until SIGINT or SIGTERM
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &mask, nullptr);
    int sig = 0;
    sigwait(&mask, &sig);
    logInfo("terminating softly after signal: %1", sig);
    return 0;
}
