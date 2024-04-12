/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/crypt/impl/botanhelper.h>

namespace Botan { namespace TLS {

class TLS12Policy : public Policy
{
public:
    TLS12Policy(bool requireRevocationInfo) : requireRevocationInfo_(requireRevocationInfo) {}

    bool acceptable_protocol_version(Protocol_Version version) const override
    {
        if (version.is_datagram_protocol()) return (version >= Protocol_Version::DTLS_V12);
        else                                return (version >= Protocol_Version::TLS_V12);
    }

    bool require_cert_revocation_info() const override
    {
        return requireRevocationInfo_;
    }

private:
    const bool requireRevocationInfo_;
};

}}
