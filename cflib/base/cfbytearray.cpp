/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "cfbytearray.h"

cfuint32 CFByteArray::toUInt(bool * ok) const {
    if (data_.empty()) { if (ok) *ok = false; return 0; }
    char * end = nullptr;
    unsigned long v = strtoul(data_.c_str(), &end, 10);
    if (ok) *ok = (end != data_.c_str() && *end == '\0');
    return (cfuint32)v;
}

cfint32 CFByteArray::toInt(bool * ok) const {
    if (data_.empty()) { if (ok) *ok = false; return 0; }
    char * end = nullptr;
    long v = strtol(data_.c_str(), &end, 10);
    if (ok) *ok = (end != data_.c_str() && *end == '\0');
    return (cfint32)v;
}
