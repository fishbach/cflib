/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/util/cmdline.h>

#include <cstdio>
#include <format>
#include <iostream>

using namespace cflib::util;

namespace {

int showUsage(const ByteArray & executable)
{
    std::cerr << std::format(
        "Usage: {} [options] <command> [args]\n"
        "Commands:\n"
        "  export <header.h> <output.cpp>  => export API definitions\n"
        "  list   <header.h>               => list API definitions\n"
        "  version                         => show version\n"
        "Options:\n"
        "  -h, --help    => this help\n",
        executable.constData());
    return 1;
}

}

int main(int argc, char *argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help('h', "help"); cmdLine << help;
    Arg command(false); cmdLine << command;
    Arg arg1; cmdLine << arg1;
    Arg arg2; cmdLine << arg2;

    if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

    if (command.value() == "export") {
        if (arg1.isSet() && arg2.isSet()) {
            std::cout << "export API from " << arg1.value().toStdString() << " to " << arg2.value().toStdString() << "\n";
            return 0;
        }
        return showUsage(cmdLine.executable());
    }

    if (command.value() == "list") {
        if (arg1.isSet()) {
            std::cout << "listing API from " << arg1.value().toStdString() << "\n";
            return 0;
        }
        return showUsage(cmdLine.executable());
    }

    if (command.value() == "version") {
        std::cout << "apiexporter 1.0.0\n";
        return 0;
    }

    return showUsage(cmdLine.executable());
}
