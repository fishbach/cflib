## Welcome to cflib

cflib is a library for Web 2.0 applications written in C++ and JavaScript and efficient network communication between C++ applications.

Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>

This file is part of cflib.
cflib is provided under the MIT license.
See the provied file COPYING for details.

# Requirements

* C++ compiler which can do C++20
* cmake >= 3.28
* Botan (comes by FetchContent, or install system-wide and use -DUSE_FETCHCONTENT=OFF)
* zlib (apt install zlib1g-dev)
* optional: Postgres (apt install libpq-dev, -DENABLE_PSQL=ON)
* optional: SQLite (apt install libsqlite3-dev, -DENABLE_SQLITE=ON)
* for writing tests with cf_test: doctest (system package or FetchContent)

# Download

```
git clone https://github.com/fishbach/cflib.git
```

# Build

```
cmake -B build
cmake --build build
ctest --test-dir build
```

By default cflib is built as **static** libraries. Build **dynamic** (shared)
libraries with `-DBUILD_SHARED_LIBS=ON`.

# Install

cflib can be installed into the system (or any prefix) to be used by other
applications:

```
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
sudo cmake --install build
```

This installs the libraries, the headers, the cmake package
(`find_package(cflib)`) and the tools (ser, apiexporter, bin2src, ...).

# Use in your own project

Either use cflib as a git submodule:

```cmake
include(3rd_party/cflib/cmake/ProjectConfig.cmake)

cf_app(myapp
    cflib_net
)
```

or use the installed package:

```cmake
find_package(cflib 1.0 REQUIRED)

cf_app(myapp
    cflib_net
)
```

Both modes provide the same `cf_lib`/`cf_app`/`cf_test` build functions and
the same target names (`cflib_base`, `cflib_net`, ..., plus `cflib::base`,
`cflib::net`, ... aliases). A project must use one mode or the other, not
both - mixing them is detected and aborted with an error.
