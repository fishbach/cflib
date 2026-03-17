/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "registerclass.h"

#include <cflib/base/macros.h>

#include <cstdio>
#include <cstdlib>

namespace cflib { namespace serialize { namespace impl {

namespace {

typedef CFHash<cfuint32, const RegisterClassBase *> Registry;
CF_GLOBAL_STATIC(Registry, getRegistry)

}

CFHash<cfuint32, const RegisterClassBase *> & RegisterClassBase::registry()
{
    return getRegistry();
}

void RegisterClassBase::duplicateId(cfuint32 classId)
{
    fprintf(stderr, "duplicate type id: %u\n", classId);
    ::abort();
}

}}}    // namespace
