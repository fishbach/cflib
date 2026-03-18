/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/ser/codegen.h>
#include <cflib/serialize/ser/headerparser.h>

#include <cstdio>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

int usage(const char * progName)
{
    std::cerr << std::format("usage: {} serialize <header.h> <source_ser.cpp>\n", progName);
    return 1;
}

int serialize(const std::string & header, const std::string & dest)
{
    // Read input file
    FILE * inFile = fopen(header.c_str(), "rb");
    if (!inFile) {
        std::cerr << std::format("cannot read: {}\n", header.c_str());
        return 2;
    }
    std::string contents;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), inFile)) > 0) {
        contents.append(buf, n);
    }
    fclose(inFile);

    HeaderParser hp;
    if (!hp.parse(contents)) {
        std::cerr << std::format("cannot parse: {} error: {}\n", header.c_str(), hp.lastError().c_str());
        return 3;
    }

    std::ofstream outFile(dest, std::ios::binary);
    if (!outFile) {
        std::cerr << std::format("cannot write file: {}\n", dest.c_str());
        return 4;
    }

    return genSerialize(header, hp, outFile);
}

int main(int argc, char *argv[])
{
    if (argc < 2) return usage(argv[0]);

    if (strcmp(argv[1], "serialize") == 0 && argc == 4) return serialize(argv[2], argv[3]);

    return usage(argv[0]);
}
