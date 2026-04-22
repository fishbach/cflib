/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "cppremoteservices.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::JS)

namespace cflib::serialize::generate {

void generateCppRemoteServices(const StructuredTypeInfos & typeInfos, const String & dest)
{
    CF_UNUSED(typeInfos);
    CF_UNUSED(dest);
}

}