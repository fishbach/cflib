/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::crypt {

class TLSStream
{
    CF_DISABLE_COPY(TLSStream)
public:
    TLSStream() {}
    virtual ~TLSStream() {}
    virtual ByteArray initialSend() = 0;
    virtual bool received(const ByteArray & encrypted, ByteArray & plain, ByteArray & sendBack) = 0;
    virtual bool send(const ByteArray & plain, ByteArray & encrypted) = 0;
};

} // namespace
