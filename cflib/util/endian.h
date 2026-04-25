
/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::util {

inline void writeBE16(uint8 * dest, uint16 val)
{
    dest[0] = (uint8)(val >> 8);
    dest[1] = (uint8)(val);
}

inline void writeBE32(uint8 * dest, uint32 val)
{
    dest[0] = (uint8)(val >> 24);
    dest[1] = (uint8)(val >> 16);
    dest[2] = (uint8)(val >> 8);
    dest[3] = (uint8)(val);
}

inline void writeBE64(uint8 * dest, uint64 val)
{
    dest[0] = (uint8)(val >> 56);
    dest[1] = (uint8)(val >> 48);
    dest[2] = (uint8)(val >> 40);
    dest[3] = (uint8)(val >> 32);
    dest[4] = (uint8)(val >> 24);
    dest[5] = (uint8)(val >> 16);
    dest[6] = (uint8)(val >> 8);
    dest[7] = (uint8)(val);
}

inline uint16 readBE16(const uint8 * src)
{
    return ((uint16)src[0] << 8) | (uint16)src[1];
}

inline uint32 readBE32(const uint8 * src)
{
    return ((uint32)src[0] << 24) | ((uint32)src[1] << 16) |
           ((uint32)src[2] << 8)  | (uint32)src[3];
}

inline uint64 readBE64(const uint8 * src)
{
    return ((uint64)src[0] << 56) | ((uint64)src[1] << 48) |
           ((uint64)src[2] << 40) | ((uint64)src[3] << 32) |
           ((uint64)src[4] << 24) | ((uint64)src[5] << 16) |
           ((uint64)src[6] << 8)  | (uint64)src[7];
}

} // namespace
