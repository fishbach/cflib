/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
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
#include <cflib/util/unixsignal.h>

#include <QtCore>

using namespace cflib::net;
using namespace cflib::util;

USE_LOG(LogCat::Etc)

int showUsage(const QByteArray & executable)
{
    QTextStream(stderr)
        << ""                                                                              << Qt::endl
        << "Simple Webchat example"                                                        << Qt::endl
        << "------------------------------------"                                          << Qt::endl
        << Qt::endl
        << "To make this example run start the webchat executable and access the url"      << Qt::endl
        << "http://127.0.0.1:8080/ through your browser."                                  << Qt::endl
        << "If you have an out of source build you need to redirect the webchat to"        << Qt::endl
        << "the correct location of the example htdocs directory through:"                 << Qt::endl
        << "./" <<executable << " --htdocs <path to cflib source>/examples/webchat/htdocs" << Qt::endl
        << Qt::endl
        << "Usage: " << executable << " [options]"                                         << Qt::endl
        << "Options:"                                                                      << Qt::endl
        << "  -h, --help     => this help"                                                 << Qt::endl
        << "  -e, --export   => export Classes as Javascrpt classes"                       << Qt::endl
        << "  -f, --htdocs   => set path to htdocs (default htdocs in current directory)"  << Qt::endl
        << Qt::endl;
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

    QCoreApplication a(argc, argv);
    UnixSignal unixSignal(true);

    Log::start("webchat.log");
    logInfo("webchat started");

    RequestLog requestLog;

    FileServer fs(htdocsPathOpt.value("htdocs"));

    WSCommManager<QString> commMgr("/ws");
    RMIServer<QString> rmiServer(commMgr);
    InfoService infoService; rmiServer.registerService(infoService);
    LogService  logService;  rmiServer.registerService(logService);

    if (exportOpt.isSet()) {
        logInfo("exporting to: %1", exportOpt.value());
        rmiServer.exportTo(exportOpt.value());
        fs.exportTo(exportOpt.value());
        return 0;
    }

    HttpServer serv;
    serv.registerHandler(requestLog);
    serv.registerHandler(commMgr);
    serv.registerHandler(rmiServer);
    serv.registerHandler(fs);
    serv.start("127.0.0.1", 8080);

    int retval = a.exec();
    logInfo("terminating softly with retval: %1", retval);
    return retval;
}
