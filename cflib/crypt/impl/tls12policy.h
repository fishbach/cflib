/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
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
