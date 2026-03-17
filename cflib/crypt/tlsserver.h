/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
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

class TLSServer : public TLSStream
{
public:
    TLSServer(TLSSessions & sessions, TLSCredentials & credentials,
        bool highSecurity = false, bool requireRevocationInfo = false);
    ~TLSServer();

    CFByteArray initialSend() override { return CFByteArray(); }
    bool received(const CFByteArray & encrypted, CFByteArray & plain, CFByteArray & sendBack) override;
    bool send(const CFByteArray & plain, CFByteArray & encrypted) override;

private:
    class Impl;
    Impl * impl_;
};

}}    // namespace
