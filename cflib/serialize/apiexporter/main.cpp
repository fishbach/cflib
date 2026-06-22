/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/generate/apidoc.h>
#include <cflib/serialize/generate/cppremoteservices.h>
#include <cflib/serialize/generate/javascript.h>
#include <cflib/serialize/generate/python.h>
#include <cflib/serialize/headerparser.h>
#include <cflib/serialize/structuredtypeinfos.h>
#include <cflib/util/cmdline.h>

#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

using namespace cflib::serialize;
using namespace cflib::serialize::generate;
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

    StructuredTypeInfos infos;
    for (ByteArray header : headers.values()) {
        if (!header.startsWith("/")) header = "../" + header;
        const String content = File::readUtf8(header);
        if (content.isNull()) {
            err << "cannot readfile: " << header.toStdString() << std::endl;
            return 1;
        }
        HeaderParser parser;
        if (!parser.parse(content)) {
            err << "cannot parse file: " << header.toStdString() << " -> " << parser.lastError().str() << std::endl;
            return 2;
        }
        for (const SerializeTypeInfo & info : parser.classes()) {
            f << "adding from " << header.toStdString() << " : " << info.toString().str() << std::endl;
            infos << info;
        }
    }

    f << "missing:" << std::endl;
    for (const SerializeTypeInfo & ti : infos.fixPlaceholders()) f << ti.ns.str() << " :: " << ti.typeName.str() << std::endl;

    f << "classes:" << std::endl;
    for (const SerializeTypeInfo & ti : infos.services() + infos.types()) f << ti.toString().str() << std::endl;

    generateCppRemoteServices(infos, "bla/chatserver/services");
    generateJavaScript       (infos, "bla/chatserver");
    generatePython           (infos, "bla/chatserver");
    generateAPIDoc           (infos, "bla", "chatserver/apidoc", "ChatServer API");

    return 0;
}
