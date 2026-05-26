# AGENTS.md

## C++ Standard

- C++20 (`CMAKE_CXX_STANDARD 20` in `cmake/ProjectConfig.cmake`)

## Build/Run Commands

```bash
cmake -B build                    # Configure (Botan fetched from source)
cmake --build build               # Build all
ctest --test-dir build            # Run tests
```

**Required flags:**
- `-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE` for clangd
- `-DENABLE_PSQL=ON` for PostgreSQL support (includes `examples/pgtest`)
- `-DENABLE_SQLITE=ON` for SQLite support
- `-DBUILD_EXAMPLES=OFF` to skip examples

**Module build order** (enforced by CMake dependency graph):
```
base → crypt → serialize → db → net, dao
```
- `net` requires `crypt` and `serialize`
- `dao` requires `db` and `serialize`

## Module-Specific Behavior

**cflib_db:** Only builds if `ENABLE_PSQL` or `ENABLE_SQLITE` is set. SQLite uses FetchContent (source: sqlite.org).

**cflib_net & cflib_dao:** Both use `ENABLE_SER` flag to auto-generate serialization code from `SERIALIZE_CLASS`/`RMIService` markers.

**cflib_serialize:** Includes `ser` codegen tool at `cflib/serialize/ser`.

## Test Framework

Tests use `cflib/util/test.h`:
- Inherit `cflib::util::TestBase`, implement `testMethods()`
- Macros: `TVERIFY(cond)`, `TCOMPARE(actual, expected)`, `QSKIP(msg)`
- Auto-register tests with `ADD_TEST(Class)` macro
- Test output: stderr; exit code 1 on any failure

## Code Generation

- `ENABLE_SER` in `cf_lib()`/`cf_app()` scans headers and generates `*_ser.cpp`
- Use `cflib/serialize/ser` tool to inspect/verify generated code
- No `gitversion` tool found; remove if stale

## Structure Conventions

- Modules: `cflib/*/` with headers + `impl/` subdirectory
- Tests: `cflib/*/*_test/` subdirectories
- Examples: `examples/*/`
- **Note:** JavaScript directory structure not verified; may be outdated

## PCH Behavior

- `cflib_base` uses `cflib/base.h` as PCH
- Other modules inherit unless they specify custom `PCH`
- Disable with `-DENABLE_PCH=OFF` (faster builds, less optimization)

## Language

- Use English only.
- No Chinese characters permitted.

## Common Mistakes

1. Running cmake only once—must re-run after adding new source files
2. Assuming all modules build by default—`db` requires `ENABLE_PSQL` or `ENABLE_SQLITE`
3. Misunderstanding `ENABLE_SER`—needs to be explicitly passed to `cf_lib()`/`cf_app()` to enable codegen
