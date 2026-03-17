/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/util.h>

#ifdef _WIN32
    #include <stdio.h>
#else
    #include <unistd.h>
#endif

int main(int, char *[])
{
    #ifdef _WIN32
        char pwd[256];
        printf("Password: ");
        gets_s(pwd, 256);
    #else
        char * pwd = getpass("Password: ");
    #endif
    CFByteArray hash = cflib::crypt::hashPassword(pwd);
    fprintf(stdout, "%s\n", hash.constData());

    return 0;
}
