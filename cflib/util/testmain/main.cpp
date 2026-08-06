/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <cflib/util/log.h>

int main(int argc, char ** argv)
{
    cflib::util::Log::start("test.log");
    return doctest::Context(argc, argv).run();
}
