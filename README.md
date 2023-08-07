## Welcome to cflib

cflib is a library for Web 2.0 applications written in C++ and JavaScript and efficient network communication between C++ applications.

Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>

This file is part of cflib.
cflib is provided under the MIT license.
See the provied file COPYING for details.

# Requirements

* C++ compiler which can do c++20
* cmake >= 3.16
* Qt >= 5.15 - https://download.qt.io/archive/qt/5.15/
* Botan >= 3.1.1 - https://botan.randombit.net/releases/

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
