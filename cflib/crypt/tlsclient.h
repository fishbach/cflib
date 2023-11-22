/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
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
    TLSClient(TLSSessions & sessions, TLSCredentials & credentials, const QByteArray & hostname = QByteArray(),
        bool highSecurity = false, bool requireRevocationInfo = false);
    ~TLSClient();

    QByteArray initialSend() override;
    bool received(const QByteArray & encrypted, QByteArray & plain, QByteArray & sendBack) override;
    bool send(const QByteArray & plain, QByteArray & encrypted) override;

private:
    class Impl;
    Impl * impl_;
};

}}    // namespace
