/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace cflib::base {

using cfint8   = std::int8_t;
using cfuint8  = std::uint8_t;
using cfint16  = std::int16_t;
using cfuint16 = std::uint16_t;
using cfint32  = std::int32_t;
using cfuint32 = std::uint32_t;
using cfint64  = std::int64_t;
using cfuint64 = std::uint64_t;
using cfuintptr = std::uintptr_t;
using cfssize  = std::ptrdiff_t;
using cfsize_t = std::size_t;
using cfuint   = unsigned int;

} // namespace
