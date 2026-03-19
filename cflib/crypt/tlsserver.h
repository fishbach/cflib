/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/crypt/tlsstream.h>

namespace cflib::crypt {

class TLSCredentials;
class TLSSessions;

class TLSServer : public TLSStream
{
public:
    TLSServer(TLSSessions & sessions, TLSCredentials & credentials,
        bool highSecurity = false, bool requireRevocationInfo = false);
    ~TLSServer();

    ByteArray initialSend() override { return ByteArray(); }
    bool received(const ByteArray & encrypted, ByteArray & plain, ByteArray & sendBack) override;
    bool send(const ByteArray & plain, ByteArray & encrypted) override;

private:
    class Impl;
    Impl * impl_;
};

} // namespace
