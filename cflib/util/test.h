/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <cmath>
#include <cstdlib>
#include <format>
#include <iostream>
#include <cstring>
#include <functional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

// ---------------------------------------------------------------------------
// Lightweight test macros
// ---------------------------------------------------------------------------

#define TVERIFY(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << std::format("  FAIL: {} at {}:{}\n", #cond, __FILE__, __LINE__); \
            cflib::util::detail::currentTestFailed() = true; \
            return; \
        } \
    } while (0)

#define TCOMPARE(actual, expected) \
    do { \
        auto _a = (actual); \
        auto _e = (expected); \
        if (!(_a == _e)) { \
            std::ostringstream _os; \
            _os << "  FAIL: " #actual " != " #expected; \
            std::cerr << std::format("{} at {}:{}\n", _os.str().c_str(), __FILE__, __LINE__); \
            cflib::util::detail::currentTestFailed() = true; \
            return; \
        } \
    } while (0)

#define QSKIP(msg) \
    do { \
        std::cout << std::format("  SKIP: {} at {}:{}\n", msg, __FILE__, __LINE__); \
        return; \
    } while (0)

#define ADD_TEST(Class) \
    namespace { \
        int cflib_util_test_add_##Class() { cflib::util::addTest(#Class, new Class); return 0; } \
        CF_CONSTRUCTOR_FUNCTION(cflib_util_test_add_##Class) \
    }

namespace cflib { namespace util {

namespace detail {
    // Per-test failure flag
    inline bool & currentTestFailed() { static bool f = false; return f; }
}

// A test case: name + list of test methods
struct TestMethod {
    const char * name;
    std::function<void()> fn;
};

class TestBase {
public:
    virtual ~TestBase() = default;
    virtual std::vector<TestMethod> testMethods() const = 0;
};

struct TestEntry {
    const char * className;
    TestBase * instance;
};

inline std::vector<TestEntry> & testRegistry() {
    static std::vector<TestEntry> reg;
    return reg;
}

inline void addTest(const char * name, TestBase * test) {
    testRegistry().push_back({name, test});
}

inline StringList allTests() {
    StringList rv;
    for (auto & e : testRegistry()) rv.push_back(String(e.className));
    return rv;
}

}}    // namespace
