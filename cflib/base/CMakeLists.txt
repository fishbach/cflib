# Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

add_library(cflib_base INTERFACE)
target_include_directories(cflib_base INTERFACE "${PROJECT_SOURCE_DIR}")
target_compile_features(cflib_base INTERFACE cxx_std_20)
