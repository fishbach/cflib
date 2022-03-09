/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "tlssessions.h"

#include <cflib/crypt/impl/botanhelper.h>

USE_LOG(LogCat::Crypt)

namespace cflib { namespace crypt {

class TLSSessions::Impl
{
public:
	Impl(bool enable) :
		mgr(enable ?
			(TLS::Session_Manager *)new TLS::Session_Manager_In_Memory(rng) :
			(TLS::Session_Manager *)new TLS::Session_Manager_Noop) {}

public:
	AutoSeeded_RNG rng;
	QSharedPointer<TLS::Session_Manager> mgr;
};

TLSSessions::TLSSessions(bool enable) :
	impl_(new Impl(enable))
{
}

TLSSessions::~TLSSessions()
{
	delete impl_;
}

Botan::TLS::Session_Manager & TLSSessions::session_Manager()
{
	return *impl_->mgr;
}

}}	// namespace
