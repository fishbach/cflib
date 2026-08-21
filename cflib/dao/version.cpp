/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "version.h"

namespace cflib::dao {

Version Version::current_;

String Version::toString() const
{
    String result = String::fromInt((uint32)major) + "." + String::fromInt((uint32)minor) + "." + String::fromInt((uint32)revision);
    if (!patchLevel.isEmpty()) result += "-" + patchLevel;
    return result;
}

void Version::setCurrent(uint major, uint minor, uint revision, const String & patchLevel)
{
    current_ = Version(major, minor, revision, patchLevel);
}

void Version::setCurrent(const Version & version)
{
    current_ = version;
}

} // namespace
