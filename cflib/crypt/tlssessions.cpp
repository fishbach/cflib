/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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

#include "tlssessions.h"

#include <cflib/crypt/impl/botanhelper.h>

USE_LOG(LogCat::Crypt)

namespace cflib { namespace crypt {

class TLSSessions::Impl
{
public:
	Impl() : mgr(rng) {}

public:
	AutoSeeded_RNG rng;
	TLS::Session_Manager_In_Memory mgr;
};

TLSSessions::TLSSessions() :
	impl_(new Impl)
{
}

TLSSessions::~TLSSessions()
{
	delete impl_;
}

Botan::TLS::Session_Manager & TLSSessions::session_Manager()
{
	return impl_->mgr;
}

}}	// namespace
