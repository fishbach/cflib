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
        for (const SerializeTypeInfo & info : parser.classes()) infos << info;
    }

    const SerializeTypeInfos missing = infos.fixPlaceholders();
    if (!missing.isEmpty()) {
        err << "Missing types:" << std::endl;
        for (const SerializeTypeInfo & ti : missing) err << "  " << ti.getName().str() << std::endl;
        return 3;
    }

    generateCppRemoteServices(infos, "chatserver/services");
    generateJavaScript       (infos, "chatserver");
    generatePython           (infos, "chatserver");
    generateAPIDoc           (infos, ".", "chatserver/apidoc", "ChatServer API");

    return 0;
}
