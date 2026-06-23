/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/structuredtypeinfos.h>

namespace cflib::serialize::generate {

void generateAPIDoc(const StructuredTypeInfos & typeInfos, const String & dest, const String & name);

} // namespace
