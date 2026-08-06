# AGENTS.md

## Quick Start

```bash
cmake -B build -DCMAKE_EXPORT_COMPILER_COMMANDS:BOOL=TRUE
cmake --build build [--parallel]
ctest [--test-dir build] [--parallel] [-R <regex>]
build/cflib/<module>/<module>_test/<module>_test [-ts=<suite>] [-tc=<case>]
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_CCACHE` | ON | Use ccache for faster builds |
| `ENABLE_PCH` | ON | Use precompiled headers |
| `ENABLE_PSQL` | OFF | Build PostgreSQL support |
| `ENABLE_SQLITE` | OFF | Build SQLite support (FetchContent) |
| `BUILD_EXAMPLES` | ON | Build example applications |
| `ENABLE_SER` | - | Enable auto-code generation in `cf_lib()`/`cf_app()`/`cf_test()` |
| `ENABLE_EXCEPTIONS` | - | Enable/disable exceptions (default: ON for crypt, OFF for others) |

## C++ Standard

C++20 (`CMAKE_CXX_STANDARD 20`, `CMAKE_CXX_STANDARD_REQUIRED ON`, `CMAKE_CXX_EXTENSIONS OFF`)

## Module Dependencies (build order)

```
cflib/base → cflib/util → cflib/serialize → cflib/crypt → cflib/db → cflib/net → cflib/dao
```

See `MODULES` file for complete dependency information.

## Module-Specific Behavior

**cflib_db:** Only builds if `ENABLE_PSQL` or `ENABLE_SQLITE` is set. SQLite uses FetchContent (source: sqlite.org).

**cflib_net & cflib_dao:** Both use `ENABLE_SER` flag to auto-generate serialization code from `SERIALIZE_CLASS` markers and RMI service code from `RMIService` markers.

**cflib_serialize:** Includes `ser` codegen tool at `cflib/serialize/ser`.

**cflib_crypt:** Uses PCH (`pch.h`), requires exceptions (`ENABLE_EXCEPTIONS`).

## CMake Macros

### `cf_lib(lib, ...)` / `cf_app(app, ...)` / `cf_test(test, lib, ...)`

See parameters in current version. Key flags: `ENABLE_EXCEPTIONS`, `ENABLE_SER`, `PCH <header>`, `DIRS <dirs>`, `OTHER_FILES <files>`.

`cf_test()` additionally takes:

| Flag | Description |
|------|-------------|
| `NO_DISCOVER` | Register the binary as one CTest test instead of one per `TEST_CASE`. Needed when cases share process state and must run in order (`db_test`). |
| `PROPERTIES <name> <value>...` | Properties set on the resulting CTest tests, e.g. `RESOURCE_LOCK <name>` to stop cases grabbing the same port from running concurrently (`net_test`). |

`cf_test()` does not take `ENABLE_EXCEPTIONS`: test targets are always built with
exceptions, because doctest compiles out the whole `REQUIRE` family without them.
That also means a test target cannot reuse the PCH of `cflib_base` (built with
`-fno-exceptions`), so it gets its own — `cflib/base.h` unless `PCH` says otherwise.

### `cf_remote(app)`

Used in `remote/` subdirectory of apps with RMI services. Creates `app_services` library linking `app_dao` and `cflib_net`.

## Code Generation (ENABLE_SER)

When `ENABLE_SER` is enabled in `cf_lib()`/`cf_app()`/`cf_test()`:
- Scans all `.h` files for `SERIALIZE_CLASS` markers
- Generates `*_ser.cpp` files in `target_autogen/`
- If `SERIALIZE_CLASS` combined with `RMIService` markers: generates RMI service handler code

Tools:
- `ser serialize <input.h> <output_ser.cpp>` - Manually generate serialization
- `bin2src <input.bin> <output_rc.cpp>` - Embed binary resources

## Test Framework

Tests use [doctest](https://github.com/doctest/doctest) (fetched via `FetchContent`,
see `cmake/Finddoctest.cmake`). Include `cflib/util/test.h`, which pulls in
`doctest/doctest.h` and teaches doctest to print `String` and `ByteArray` values in
failure output. `main()` comes from `cflib_testmain`, which `cf_test()` links
automatically; it starts the log (`test.log`) and hands over to doctest.

```cpp
#include <cflib/util/test.h>

TEST_SUITE("ByteArray") {

TEST_CASE("ByteArray: split")
{
    REQUIRE(!ByteArray("a,b").isNull());
    REQUIRE_EQ(ByteArray("a,b").split(',').size(), (size_t)2);
}

}
```

Assertions: `REQUIRE(cond)` / `REQUIRE_EQ(actual, expected)` abort the test case on
failure; the `CHECK*` variants report and continue. To skip at runtime, use
`MESSAGE("SKIP: ...")` followed by `return`.

Conventions:

- One `TEST_SUITE` per file, named after the type or subject under test.
- `TEST_CASE` names are `"<Subject>: <what>"`. CTest names come from the test case
  name alone (test suites are not part of it), so names must be unique per binary.
- Wrap `&&` / `||` expressions in an extra pair of parentheses — doctest cannot
  decompose them and fails with "Expression Too Complex".

Running:

```bash
ctest --test-dir build --parallel        # one CTest entry per TEST_CASE
build/cflib/base/base_test/base_test --list-test-cases
build/cflib/base/base_test/base_test -ts="ByteArray"     # whole suite
build/cflib/base/base_test/base_test -tc="ByteArray: split"
```

## Structure Conventions

- Modules: `cflib/*/` with headers + `impl/` subdirectory
- Tests: `cflib/*/*_test/` subdirectories
- Examples: `examples/*/` (see `MODULES` for dependencies)

## Build Tools

- `ser` - serialization codegen (`cflib/serialize/ser`)
- `bin2src` - binary resource embedding (`cflib/util/bin2src`)
- `jscombiner` - JavaScript file combiner (`cflib/util/jscombiner`)
- `migrationmoped` - database migration tool (`cflib/db/migrationmoped`)
- `asn1dump` - ASN.1 dump utility (`cflib/serialize/asn1dump`)

## Common Mistakes

1. **Running cmake only once**—must re-run after adding new source files
2. **Assuming all modules build by default**—`db` requires `ENABLE_PSQL` or `ENABLE_SQLITE`
3. **Misunderstanding `ENABLE_SER`**—needs to be explicitly passed to `cf_lib()`/`cf_app()`/`cf_test()` to enable codegen
4. **Typo in compile commands flag**—use `CMAKE_EXPORT_COMPILER_COMMANDS`, not `CMAKE_EXPORT_COMPILE_COMMANDS`
5. **Wrong module dependency order**—see `MODULES` file (uses underscores: `cflib_base`, not dots: `cflib.base`)
