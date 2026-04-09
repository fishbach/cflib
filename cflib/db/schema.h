/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <functional>

namespace cflib::db::schema {

typedef std::function<bool (const ByteArray & name)> Migrator;

template<typename S>
bool update(Migrator migrator = Migrator(), const String & filename = ":/schema.sql");
template<typename S>
bool update(const ByteArray & schema, Migrator migrator = Migrator());

template<typename S, typename M>
bool update(const String & filename = ":/schema.sql")
{
    M migrator;
    return update<S>(
        [&migrator](const ByteArray & name) { return migrator.migrate(name); },
        filename);
}

} // namespace
