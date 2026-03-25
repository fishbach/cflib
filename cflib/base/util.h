/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/container.h>
#include <cflib/base/string.h>

namespace cflib::base {

using StringList = List<String>;

template<typename T>
inline T max(const T & a, const T & b) { return a > b ? a : b; }

} // namespace
