/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/util.h>
#include <cflib/util/hex.h>

namespace cflib { namespace serialize {

QString printAsn1(const QByteArray & data);

}}	// namespace
