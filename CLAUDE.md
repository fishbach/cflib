# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

cflib is a C++ and JavaScript library for Web 2.0 applications with efficient network communication. It is a standalone C++ library (Qt was recently removed) requiring C++20, CMake >= 3.16, Botan >= 3.1.1, and zlib.

## Build Commands

```bash
cmake -B build                    # Configure
cmake --build build               # Build all
cmake --build build -j$(nproc)   # Build with parallelism
ctest --test-dir build            # Run all tests
ctest --test-dir build -R util    # Run specific test suite
./build/bin/util_test             # Run test binary directly
./build/bin/util_test ClassName   # Run specific test class
```

Optional CMake flags:
- `-DBUILD_EXAMPLES=OFF` — skip examples
- `-DENABLE_PSQL=ON` — enable PostgreSQL support

## Module Architecture

Modules must respect this dependency order (no cycles):

```
cflib/util          (no deps — core utilities: strings, byte arrays, threading, compression, event loop)
cflib/crypt         -> cflib/util         (TLS/SSL via Botan, password hashing)
cflib/serialize     -> cflib/util         (BER encoding, schema, ASN.1)
cflib/db            -> cflib/util         (connection pooling, schema migration; optional PostgreSQL)
cflib/dao           -> cflib/db, cflib/serialize   (ORM-like data access, auto-serialization)
cflib/net           -> cflib/crypt, cflib/serialize (TCP/HTTP/WebSocket, message serialization)
cflib/base          (interface-only: sets C++20 standard and include paths)
```

Each module has an `impl/` subdirectory for implementation files and a `*_test/` subdirectory for tests.

## CMake Build Functions

Defined in `cmake/Build.cmake`:

- `cf_lib(name PUBLIC deps... PRIVATE deps... DIRS subdirs... PCH file ENABLE_SER)` — define a library
- `cf_app(name DEPS deps... DIRS subdirs...)` — define an executable
- `cf_test(name DEPS deps... DIRS subdirs...)` — define a test executable (auto-registered with CTest)

`ENABLE_SER` triggers auto-generation of `*_ser.cpp` files by scanning headers for `SERIALIZE_CLASS`.

## Test Framework

Tests use a custom lightweight framework (in `cflib/util/test.h`) that replaced QtTest. Test classes inherit `cflib::util::TestBase` and implement `testMethods()`:

```cpp
class MyTest : public cflib::util::TestBase {
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<MyTest *>(this);
        return {
            {"myTest", [self]() { self->myTest(); }},
        };
    }
    void myTest() {
        QVERIFY(condition);
        QCOMPARE(actual, expected);
    }
};
```

Test output goes to `test.log`; exit code is 1 on failure.

## Precompiled Headers

PCH files are in `pch/`. Each library can specify `PCH pch/util.h` in `cf_lib()`. Without a custom PCH, the default `pch_core` library is used. Botan is fetched and built from source by CMake (see `cmake/FindBotan.cmake`) on first configure — this takes time.

## JavaScript

`js/` mirrors the C++ module structure (domext, net, util) for browser-side code. The `jscombiner` tool combines JS files for deployment.

## Utility Tools

- `cflib/util/gitversion` — embeds git version info into builds
- `cflib/util/jscombiner` — combines JS files
- `cflib/serialize/ser` — serialization code generator
- `cflib/serialize/asn1dump` — ASN.1 inspection
- `cflib/crypt/pwhash` — password hashing CLI
