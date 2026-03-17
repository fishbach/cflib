/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "test.h"

#include <cflib/util/log.h>

#include <algorithm>
#include <cstring>

using namespace cflib::util;

int main(int argc, char *argv[])
{
    const char * runOnly = nullptr;
    if (argc > 1 && argv[1][0] != '-') {
        runOnly = argv[1];
        // Check if class exists
        bool found = false;
        for (auto & e : testRegistry()) {
            if (strcasecmp(e.className, runOnly) == 0) { found = true; break; }
        }
        if (!found) {
            fprintf(stderr, "Class \"%s\" not found!\nExisting classes:\n", argv[1]);
            for (auto & e : testRegistry()) fprintf(stderr, "  %s\n", e.className);
            return 1;
        }
    }

    Log::start("test.log");

    int failures = 0;
    int passed = 0;

    for (auto & entry : testRegistry()) {
        if (runOnly && strcasecmp(entry.className, runOnly) != 0) continue;

        fprintf(stdout, "--- %s ---\n", entry.className);
        auto methods = entry.instance->testMethods();
        for (auto & method : methods) {
            detail::currentTestFailed() = false;
            method.fn();
            if (detail::currentTestFailed()) {
                fprintf(stdout, "  FAIL: %s::%s\n", entry.className, method.name);
                ++failures;
            } else {
                fprintf(stdout, "  PASS: %s::%s\n", entry.className, method.name);
                ++passed;
            }
        }
    }

    fprintf(stdout, "\nTotal: %d passed, %d failed\n", passed, failures);
    return failures > 0 ? 1 : 0;
}
