/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfcontainers.h>
#include <cflib/base/macros.h>

namespace cflib { namespace net {

// Attention: this function may need some time (blocks)
CFList<CFByteArray> getIPFromDNS(const CFByteArray & name, bool preferIPv6 = false);

}}    // namespace
