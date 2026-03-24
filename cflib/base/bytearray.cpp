/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "bytearray.h"

namespace cflib::base {

uint32 ByteArray::toUInt(bool * ok) const {
    if (d->data.empty()) { if (ok) *ok = false; return 0; }
    char * end = nullptr;
    unsigned long v = strtoul(d->data.c_str(), &end, 10);
    if (ok) *ok = (end != d->data.c_str() && *end == '\0');
    return (uint32)v;
}

int32 ByteArray::toInt(bool * ok) const {
    if (d->data.empty()) { if (ok) *ok = false; return 0; }
    char * end = nullptr;
    long v = strtol(d->data.c_str(), &end, 10);
    if (ok) *ok = (end != d->data.c_str() && *end == '\0');
    return (int32)v;
}

} // namespace
