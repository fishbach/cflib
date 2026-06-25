/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/fileserver/fileserverbase.h>
#include <cflib/util/cmdline.h>

#include <iostream>

using namespace cflib::net::fileserver;
using namespace cflib::util;

namespace {

auto && err = std::cerr;

int showUsage(const ByteArray & executable)
{
    err << "Usage: " << executable.toStdString() << " <src> <dest>" << std::endl
        << "Options:"                                               << std::endl
        << "  -h, --help  => this help"                             << std::endl;
    return 1;
}

}

int main(int argc, char *argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help('h', "help"); cmdLine << help;
    Arg srcDest(false, true); cmdLine << srcDest;
    if (!cmdLine.parse() || help.isSet() || srcDest.count() != 2) return showUsage(cmdLine.executable());

    FileServerBase fs(srcDest[0]);
    fs.exportTo(srcDest[1]);

    return 0;
}
