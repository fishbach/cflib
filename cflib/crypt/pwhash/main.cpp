/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/util.h>

#include <format>
#include <iostream>

#ifdef _WIN32
    #include <stdio.h>
#else
    #include <unistd.h>
#endif

int main(int argc, char * argv[])
{
    #ifdef _WIN32
        char pwd[256];
        std::cout << "Password: " << std::flush;
        gets_s(pwd, 256);
    #else
        char * pwd = getpass("Password: ");
    #endif

    uint16 workFactor = 12;
    if (argc == 2) workFactor = String(argv[1]).toUInt();

    ByteArray hash = cflib::crypt::hashPassword(pwd, workFactor);
    std::cout << std::format("{}\n", std::string(hash.constData(), hash.size()));

    return 0;
}
