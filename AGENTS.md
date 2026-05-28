# AGENTS.md

## Quick Start

```bash
# Configure (Botan fetched from source, ccache enabled by default)
cmake -B build -DCMAKE_EXPORT_COMPILER_COMMANDS:BOOL=TRUE

# Build
cmake --build build [--parallel]

# Run tests
ctest [--test-dir build] [--parallel] [-R <regex>]

# Run specific test class
./bin/<test_name> <ClassName>
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_CCACHE` | ON | Use ccache for faster builds |
| `ENABLE_PCH` | ON | Use precompiled headers |
| `ENABLE_PSQL` | OFF | Build PostgreSQL support (includes `examples/pgtest`) |
| `ENABLE_SQLITE` | OFF | Build SQLite support (uses FetchContent from sqlite.org) |
| `BUILD_EXAMPLES` | ON | Build example applications |
| `ENABLE_SER` | - | Enable auto-code generation in `cf_lib()`/`cf_app()`/`cf_test()` |
| `ENABLE_GIT_VERSION` | - | Generate gitversion.h (use with `cf_app()`) |
| `ENABLE_EXCEPTIONS` | - | Enable/disable exceptions (default: ON for crypt, OFF for others) |
| `CF_INTERN` | - | Internal tools (output to build dir, not `bin/`) |

## C++ Standard

- C++20 (`CMAKE_CXX_STANDARD 20`, `CMAKE_CXX_STANDARD_REQUIRED ON`, `CMAKE_CXX_EXTENSIONS OFF` in `cmake/ProjectConfig.cmake`)

## Module Dependencies

See `MODULES` file for complete dependency information.

**Build order** (from dependencies):
```
cflib/base → cflib/util → cflib/serialize → cflib/crypt → cflib/db → cflib/net → cflib/dao
```

- `cflib_base` has no dependencies
- `cflib_util` requires `cflib_base`
- `cflib_crypt` requires `cflib_util`
- `cflib_serialize` requires `cflib_util`
- `cflib_db` requires `cflib_util`
- `cflib_net` requires `cflib_crypt` + `cflib_serialize`
- `cflib_dao` requires `cflib_db` + `cflib_serialize`

## Module-Specific Behavior

**cflib_db:** Only builds if `ENABLE_PSQL` or `ENABLE_SQLITE` is set. SQLite uses FetchContent (source: sqlite.org).

**cflib_net & cflib_dao:** Both use `ENABLE_SER` flag to auto-generate serialization code from `SERIALIZE_CLASS` markers and RMI service code from `RMIService` markers.

**cflib_serialize:** Includes `ser` codegen tool at `cflib/serialize/ser`.

**cflib_crypt:** Uses PCH (`pch.h`), requires exceptions (`ENABLE_EXCEPTIONS`).

## CMake Macros

### `cf_lib(lib, ...)`
Create a library. Parameters:
- `ENABLE_EXCEPTIONS` - enable exceptions
- `ENABLE_SER` - enable code generation
- `PCH <header>` - custom PCH header
- `PUBLIC <deps>` / `PRIVATE <deps>` - library dependencies
- `DIRS <dirs>` - source directories
- `OTHER_FILES <files>` - non-source files to include in project

### `cf_app(app, ...)`
Create an executable. Parameters:
- `ENABLE_EXCEPTIONS` - enable exceptions
- `ENABLE_SER` - enable code generation
- `ENABLE_GIT_VERSION` - generate gitversion.h
- `CF_INTERN` - internal tool (no bin/ output)
- `PCH <header>` - custom PCH header
- `DAO <dir>` - DAO source directory
- `DIRS <dirs>` - source directories
- `RESOURCES <files>` - binary resources to embed
- `OTHER_FILES <files>` - non-source files

### `cf_test(test, lib, ...)`
Create a test executable. Parameters:
- `ENABLE_EXCEPTIONS` - enable exceptions
- `ENABLE_SER` - enable code generation
- `PCH <header>` - custom PCH header
- `DIRS <dirs>` - source directories
- `RESOURCES <files>` - binary resources

## Code Generation

### Auto-Serialization (ENABLE_SER)
When `ENABLE_SER` is enabled in `cf_lib()`/`cf_app()`/`cf_test()`:
- Scans all `.h` files for `SERIALIZE_CLASS` markers
- Generates `*_ser.cpp` files in `target_autogen/`
- Generated code implements serialization/deserialization

### RMI Service Generation
When `SERIALIZE_CLASS` is combined with `RMIService` markers:
- Scans for RMI service definitions
- Generates service handler code

### Tools
- `ser serialize <input.h> <output_ser.cpp>` - Manually generate serialization
- `bin2src <input.bin> <output_rc.cpp>` - Embed binary resources
- `gitversion create <source_dir> <header.h>` - Generate version header

## Remote Interface Generation

For applications with RMI services:

**Project Structure:**
```
myapp/
  CMakeLists.txt
  services/
  dao/
  remote/     # Generated files go here
    CMakeLists.txt
```

**CMake Configuration:**
```cmake
# main CMakeLists.txt
cf_app(myapp
    cflib_net
    DIRS services
    DAO dao
    ENABLE_SER
)
add_subdirectory(remote)

# remote/CMakeLists.txt
cf_remote(myapp)
```

This will:
1. Build `myapp` with `ENABLE_SER` and `DAO`
2. Run `myapp --export remote/` to generate service files
3. Create `myapp_services` library linking `myapp_dao` and `cflib_net`

## Test Framework

### Writing Tests
Test executables use `cflib/util/test.h`:

- Inherit `cflib::util::TestBase`
- Implement `std::vector<TestMethod> testMethods() const override`
- Return list of method names and lambdas
- Auto-register tests with `ADD_TEST(Class)` macro

### Test Macros
- `TVERIFY(cond)` - Verify condition, output to stderr
- `TCOMPARE(actual, expected)` - Compare with `operator==`, output to stderr
- `QSKIP(msg)` - Skip test method

### Running Tests
- `ctest` runs all tests
- `./bin/test_name` lists all available test classes
- `./bin/test_name ClassName` runs specific class only

**Example:**
```cpp
#include <cflib/util/test.h>

class My_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<My_Test *>(this);
        return {
            {"test_something", [self]() { self->test_something(); }},
        };
    }

    void test_something() {
        TVERIFY(1 == 1);
    }
};
ADD_TEST(My_Test)
```

## Configuration Files

CMake searches upward from project root for `config.cmake` files:
- Each found file is loaded in directory order (root to project)
- Allows project-wide and per-directory configuration
- Example: project root could have `config.cmake` with defaults

## PCH Behavior

- `cflib_base` uses `cflib/base.h` as custom PCH
- Other modules reuse from `cflib_base` unless they specify custom `PCH`
- Disable with `-DENABLE_PCH=OFF`

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

## Language

- Use English only.
- No Chinese characters permitted.

## Common Mistakes

1. **Running cmake only once**—must re-run after adding new source files
2. **Assuming all modules build by default**—`db` requires `ENABLE_PSQL` or `ENABLE_SQLITE`
3. **Misunderstanding `ENABLE_SER`**—needs to be explicitly passed to `cf_lib()`/`cf_app()`/`cf_test()` to enable codegen
4. **Typo in compile commands flag**—use `CMAKE_EXPORT_COMPILER_COMMANDS`, not `CMAKE_EXPORT_COMPILE_COMMANDS`
5. **Forgetting gitversion tool exists**—it exists and is used with `ENABLE_GIT_VERSION`
6. **Missing test filtering**—run `./bin/test_name ClassName` to run specific tests
7. **Wrong module dependency order**—see `MODULES` file for correct order (uses underscores: `cflib_base`, not dots: `cflib.base`)
