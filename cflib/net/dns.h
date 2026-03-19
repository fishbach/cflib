/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::net {

// Attention: this function may need some time (blocks)
CFList<ByteArray> getIPFromDNS(const ByteArray & name, bool preferIPv6 = false);

} // namespace
