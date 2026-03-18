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

int main(int, char *[])
{
    #ifdef _WIN32
        char pwd[256];
        std::cout << "Password: " << std::flush;
        gets_s(pwd, 256);
    #else
        char * pwd = getpass("Password: ");
    #endif
    CFByteArray hash = cflib::crypt::hashPassword(pwd);
    std::cout << std::format("{}\n", hash.constData());

    return 0;
}
