/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/crypt/tlsstream.h>

namespace cflib { namespace crypt {

class TLSCredentials;
class TLSSessions;

class TLSClient : public TLSStream
{
public:
    TLSClient(TLSSessions & sessions, TLSCredentials & credentials, const ByteArray & hostname = ByteArray(),
        bool highSecurity = false, bool requireRevocationInfo = false);
    ~TLSClient();

    ByteArray initialSend() override;
    bool received(const ByteArray & encrypted, ByteArray & plain, ByteArray & sendBack) override;
    bool send(const ByteArray & plain, ByteArray & encrypted) override;

private:
    class Impl;
    Impl * impl_;
};

}}    // namespace
