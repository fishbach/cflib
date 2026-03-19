/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/util.h>
#include <cflib/util/hex.h>

namespace cflib { namespace serialize {

String printAsn1(const CFByteArray & data);

}}    // namespace
