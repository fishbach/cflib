/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfbytearray.h>
#include <cflib/base/cfstring.h>

#include <functional>

namespace cflib { namespace db { namespace schema {

typedef std::function<bool (const CFByteArray & name)> Migrator;

bool update(Migrator migrator, const CFString & filename);
bool update(const CFByteArray & schema, Migrator migrator = Migrator());

template<typename M>
bool update(const CFString & filename = ":/schema.sql")
{
    M migrator;
    return update(
        [&migrator](const CFByteArray & name) { return migrator.migrate(name); },
        filename);
}

}}}    // namespace
