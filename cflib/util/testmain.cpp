/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/util/test.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

using namespace cflib::util;

int main(int argc, char * argv[])
{
    const char * runOnly = nullptr;
    if (argc > 1 && argv[1][0] != '-') {
        runOnly = argv[1];
        bool found = false;
        for (const auto & e : testRegistry()) {
            if (std::strcmp(e.className, runOnly) == 0) { found = true; break; }
        }
        if (!found) {
            fprintf(stderr, "Class \"%s\" not found!\nExisting classes:\n", runOnly);
            for (const auto & e : testRegistry()) fprintf(stderr, "  %s\n", e.className);
            return 1;
        }
    }

    int failures = 0;
    bool first = true;
    for (const auto & e : testRegistry()) {
        if (runOnly && std::strcmp(e.className, runOnly) != 0) continue;
        if (first) first = false;
        else fprintf(stdout, "\n");
        fprintf(stdout, "********* Start testing %s *********\n", e.className);
        bool classOk = true;
        for (const auto & m : e.instance->testMethods()) {
            detail::currentTestFailed() = false;
            m.fn();
            if (detail::currentTestFailed()) {
                ++failures;
                classOk = false;
            } else {
                fprintf(stdout, "  PASS: %s\n", m.name);
            }
        }
        if (classOk) fprintf(stdout, "PASSED\n");
        else         fprintf(stdout, "FAILED\n");
        fprintf(stdout, "********* Finished testing %s *********\n", e.className);
    }
    return failures > 0 ? 1 : 0;
}
