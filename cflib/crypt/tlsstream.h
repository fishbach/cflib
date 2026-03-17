/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfbytearray.h>
#include <cflib/base/macros.h>

namespace cflib { namespace crypt {

class TLSStream
{
    CF_DISABLE_COPY(TLSStream)
public:
    TLSStream() {}
    virtual ~TLSStream() {}
    virtual CFByteArray initialSend() = 0;
    virtual bool received(const CFByteArray & encrypted, CFByteArray & plain, CFByteArray & sendBack) = 0;
    virtual bool send(const CFByteArray & plain, CFByteArray & encrypted) = 0;
};

}}    // namespace
