# AGENTS.md

## Quick Start

```bash
cmake -B build -DCMAKE_EXPORT_COMPILER_COMMANDS:BOOL=TRUE
cmake --build build [--parallel]
ctest [--test-dir build] [--parallel] [-R <regex>]
./bin/<test_name> <ClassName>
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
| `ENABLE_GIT_VERSION` | - | Generate gitversion.h (use with `cf_app()`) |
| `ENABLE_EXCEPTIONS` | - | Enable/disable exceptions (default: ON for crypt, OFF for others) |
| `CF_INTERN` | - | Internal tools (output to build dir, not `bin/`) |

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
- `gitversion create <source_dir> <header.h>` - Generate version header

## Test Framework

Test executables use `cflib/util/test.h`:

- Inherit `cflib::util::TestBase`
- Implement `std::vector<TestMethod> testMethods() const override`
- Return list of method names and lambdas
- Auto-register tests with `ADD_TEST(Class)` macro

Macros:
- `TVERIFY(cond)` - Verify condition, output to stderr
- `TCOMPARE(actual, expected)` - Compare with `operator==`, output to stderr
- `QSKIP(msg)` - Skip test method

## Structure Conventions

- Modules: `cflib/*/` with headers + `impl/` subdirectory
- Tests: `cflib/*/*_test/` subdirectories
- Examples: `examples/*/` (see `MODULES` for dependencies)
- Binary output: `bin/` (except with `CF_INTERN` flag)

## Build Tools

- `ser` - serialization codegen (`cflib/serialize/ser`)
- `bin2src` - binary resource embedding (`cflib/util/bin2src`)
- `jscombiner` - JavaScript file combiner (`cflib/util/jscombiner`)
- `gitversion` - git version header generation (`cflib/util/gitversion`)
- `migrationmoped` - database migration tool (`cflib/db/migrationmoped`)
- `asn1dump` - ASN.1 dump utility (`cflib/serialize/asn1dump`)

## Common Mistakes

1. **Running cmake only once**—must re-run after adding new source files
2. **Assuming all modules build by default**—`db` requires `ENABLE_PSQL` or `ENABLE_SQLITE`
3. **Misunderstanding `ENABLE_SER`**—needs to be explicitly passed to `cf_lib()`/`cf_app()`/`cf_test()` to enable codegen
4. **Typo in compile commands flag**—use `CMAKE_EXPORT_COMPILER_COMMANDS`, not `CMAKE_EXPORT_COMPILE_COMMANDS`
5. **Wrong module dependency order**—see `MODULES` file (uses underscores: `cflib_base`, not dots: `cflib.base`)
