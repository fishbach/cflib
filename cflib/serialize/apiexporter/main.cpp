/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/headerparser.h>
#include <cflib/util/cmdline.h>

#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

using namespace cflib::serialize;
using namespace cflib::util;

namespace {

auto && err = std::cerr;

int showUsage(const ByteArray & executable)
{
    err << "Usage: " << executable.toStdString() << " [options] <headers>"  << std::endl
        << "Options:"                                                       << std::endl
        << "  -h, --help        => this help"                               << std::endl
        << "  -t, --type <type> => type of API to be exported (repeatable)" << std::endl
        << "API Types:"                                                     << std::endl
        << "  apidoc cpp javascript python"                                 << std::endl;
    return 1;
}

}

int main(int argc, char *argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help   ('h', "help"                   ); cmdLine << help;
    Option types  ('t', "type", true, false, true); cmdLine << types;
    Arg    headers(false, true                   ); cmdLine << headers;
    if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

    std::ofstream f{"dudi"};
    f << "output types:";
    for (const ByteArray & type : types.values()) {
        f << " " << type.toStdString();
    }
    f << std::endl;
    f << "sources:";
    for (const ByteArray & header : headers.values()) {
        f << " " << header.toStdString();
    }
    f << std::endl;

    HeaderParser parser;
    for (const ByteArray & header : headers.values()) {

    }

    return 0;
}
