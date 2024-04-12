/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace Botan { namespace TLS { class Session_Manager; }}

namespace cflib { namespace crypt {

class TLSSessions
{
    Q_DISABLE_COPY(TLSSessions)
public:
    TLSSessions(bool enable = false);
    ~TLSSessions();

private:
    class Impl;
    Impl * impl_;

    friend class TLSClient;
    friend class TLSServer;
    Botan::TLS::Session_Manager & session_Manager();
};

}}    // namespace
