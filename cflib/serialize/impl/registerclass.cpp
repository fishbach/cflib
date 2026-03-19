/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "registerclass.h"

#include <cflib/base.h>

#include <cstdlib>
#include <format>
#include <iostream>

namespace cflib::serialize::impl {

namespace {

typedef Hash<uint32, const RegisterClassBase *> Registry;
CF_GLOBAL_STATIC(Registry, getRegistry)

}

Hash<uint32, const RegisterClassBase *> & RegisterClassBase::registry()
{
    return getRegistry();
}

void RegisterClassBase::duplicateId(uint32 classId)
{
    std::cerr << std::format("duplicate type id: {}\n", classId);
    ::abort();
}

} // namespace
