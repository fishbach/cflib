/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "version.h"

namespace cflib { namespace dao {

Version Version::current_;

QString Version::toString() const
{
    return QString("%1.%2.%3%4")
        .arg(major).arg(minor).arg(revision)
        .arg(patchLevel.isEmpty() ? "" : "-" + patchLevel);
}

void Version::setCurrent(uint major, uint minor, uint revision, const QString & patchLevel)
{
    current_ = Version(major, minor, revision, patchLevel);
}

void Version::setCurrent(const Version & version)
{
    current_ = version;
}

}}
