/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <functional>

namespace cflib { namespace db { namespace schema {

typedef std::function<bool (const CFByteArray & name)> Migrator;

bool update(Migrator migrator, const String & filename);
bool update(const CFByteArray & schema, Migrator migrator = Migrator());

template<typename M>
bool update(const String & filename = ":/schema.sql")
{
    M migrator;
    return update(
        [&migrator](const CFByteArray & name) { return migrator.migrate(name); },
        filename);
}

}}}    // namespace
