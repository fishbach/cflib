/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "test.h"

#include <cflib/util/log.h>

#include <algorithm>
#include <cstring>
#include <format>
#include <iostream>

using namespace cflib::util;

int main(int argc, char *argv[])
{
    const char * runOnly = nullptr;
    if (argc > 1 && argv[1][0] != '-') {
        runOnly = argv[1];
        bool found = false;
        for (auto & e : testRegistry()) {
            if (strcasecmp(e.className, runOnly) == 0) { found = true; break; }
        }
        if (!found) {
            std::cerr << std::format("Class \"{}\" not found!\nExisting classes:\n", argv[1]);
            for (auto & e : testRegistry()) std::cerr << std::format("  {}\n", e.className);
            return 1;
        }
    }

    Log::start("test.log");

    int failures = 0;
    int passed = 0;

    for (auto & entry : testRegistry()) {
        if (runOnly && strcasecmp(entry.className, runOnly) != 0) continue;

        std::cout << std::format("--- {} ---\n", entry.className);
        auto methods = entry.instance->testMethods();
        for (auto & method : methods) {
            detail::currentTestFailed() = false;
            method.fn();
            if (detail::currentTestFailed()) {
                std::cout << std::format("  FAIL: {}::{}\n", entry.className, method.name);
                ++failures;
            } else {
                std::cout << std::format("  PASS: {}::{}\n", entry.className, method.name);
                ++passed;
            }
        }
    }

    std::cout << std::format("\nTotal: {} passed, {} failed\n", passed, failures);
    return failures > 0 ? 1 : 0;
}
