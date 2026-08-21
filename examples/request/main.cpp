/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/tlscredentials.h>
#include <cflib/net/httprequest.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <cstdio>
#include <format>
#include <iostream>
#include <unistd.h>

using namespace cflib::net;
using namespace cflib::util;

USE_LOG(LogCat::Network)

namespace {

int showUsage(const ByteArray & executable)
{
    std::cerr << std::format(
        "Usage: {} [options] <URL>\n"
        "Options:\n"
        "  -h, --help        => this help\n"
        "  -p, --post        => do POST request\n"
        "  -l, --log <level> => set log level 0 -> all, 7 -> off\n"
        "  -c, --certs <dir> => loads root certificates from given directory\n"
        "\n"
        "Get the root ca of a certain domain:\n"
        "  openssl s_client -showcerts -servername cflib.de -connect cflib.de:443 < /dev/null\n"
        "  paste last certificate to: openssl x509 -noout -text\n"
        "  curl http://crl.identrust.com/DSTROOTCAX3CRL.crl | openssl crl -inform DER\n"
        "  curl http://apps.identrust.com/roots/dstrootcax3.p7c | openssl pkcs7 -inform DER -print_certs\n",
        executable.constCharPtr());
    return 1;
}

}

int main(int argc, char *argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help     ('h', "help"       ); cmdLine << help;
    Option postOpt  ('p', "post"       ); cmdLine << postOpt;
    Option logOpt   ('l', "log",   true); cmdLine << logOpt;
    Option certsOpt ('c', "certs", true); cmdLine << certsOpt;
    Arg     url     (false             ); cmdLine << url;
    if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

    // start logging
    if (logOpt.isSet()) {
        Log::start("request.log");
        logInfo("request started");
        Log::setLogLevel(logOpt.value().toUInt());
    }

    ByteArray postData;
    if (postOpt.isSet()) {
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0)
            postData.append(buf, n);
    }

    TCPManager mgr(1);

    if (certsOpt.isSet()) {
        mgr.clientCredentials().loadFromDir(String(certsOpt.value()));
        mgr.clientCredentials().activateLoaded(true);
    }

    HttpRequest * request = new HttpRequest(mgr);
    request->reply.bind([](int status, const ByteArray & reply) {
        std::cout << std::format("Status: {}\n\n", status);
        fwrite(reply.constCharPtr(), 1, reply.size(), stdout);
        std::cout << "\n";
        threadSafeExit(status == 200 ? 0 : 1);
    });
    request->start(Url(String(url.value())), postData);

    // Block main thread; threadSafeExit() will call _exit() from the reply callback
    for (;;) pause();
}
