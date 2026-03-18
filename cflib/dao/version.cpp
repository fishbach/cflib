/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "version.h"

namespace cflib { namespace dao {

Version Version::current_;

CFString Version::toString() const
{
    CFString result = CFString::number((cfuint32)major) + "." + CFString::number((cfuint32)minor) + "." + CFString::number((cfuint32)revision);
    if (!patchLevel.isEmpty()) result += "-" + patchLevel;
    return result;
}

void Version::setCurrent(cfuint major, cfuint minor, cfuint revision, const CFString & patchLevel)
{
    current_ = Version(major, minor, revision, patchLevel);
}

void Version::setCurrent(const Version & version)
{
    current_ = version;
}

}}
