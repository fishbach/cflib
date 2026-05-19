# AGENTS.md

## C++ Standard

- Der verwendete C++ Standard ist C++20.

## Module Dependency Order

Build modules in this order (enforced by `cflib/CMakeLists.txt`):
```
base → crypt → serialize → db → net, dao
```
`dao` requires `db` and `serialize`; `net` requires `crypt` and `serialize`. No cycles permitted.

## Build/Run Commands

```bash
cmake -B build                    # Configure (Botan fetched from source on first run)
cmake --build build               # Build all
ctest --test-dir build            # Run all tests
```

Optional flags:
- `-DBUILD_EXAMPLES=OFF` — skip examples
- `-DENABLE_PSQL=ON` — enable PostgreSQL support (builds `examples/pgtest`)
- `-DENABLE_SQLITE=ON` — enable SQLite support
- `-DENABLE_PCH=OFF` — disable precompiled headers (builds faster but less optimized)

clangd:
- For clangd you need the parameter `-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE`

## Test Framework

- Tests in `cflib/util/test.h`: inherit `cflib::util::TestBase`, implement `testMethods()`
- Use `TVERIFY(cond)` and `TCOMPARE(actual, expected)` macros
- Test output goes to `test.log`; exit code 1 on failure
- Test classes auto-registered via `ADD_TEST(Class)` macro

## Code Generation

- `ENABLE_SER` in `cf_lib()`/`cf_app()` scans headers for `SERIALIZE_CLASS`/`RMIService` and auto-generates `*_ser.cpp`
- `ser` tool at `cflib/serialize/ser` handles serialization codegen
- `gitversion` tool embeds git info when `ENABLE_GIT_VERSION` is set

## Structure Conventions

- Each module: `cflib/*/` contains headers (public API), `impl/` (private implementation)
- Tests: `cflib/*/*_test/` subdirectories
- JavaScript mirrors C++ modules: `js/{domext,net,util}/`

## PCH Behavior

- `cflib_base` uses `cflib/base.h` as PCH
- Other modules inherit from `cflib_base` unless they specify custom `PCH`

## Language

- Use English only.
- Do not use special characters.

## New Source Files

- When new source files need to be created, make sure to call cmake manually, to that new compile units will be created.
