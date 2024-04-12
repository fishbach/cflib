/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>
#include <functional>

namespace cflib { namespace db { namespace schema {

typedef std::function<bool (const QByteArray & name)> Migrator;

bool update(QObject * migrator = 0, const QString & filename = ":/schema.sql");
bool update(Migrator migrator     , const QString & filename = ":/schema.sql");
bool update(const QByteArray & schema, QObject * migrator = 0);
bool update(const QByteArray & schema, Migrator migrator);

template<typename M>
bool update(const QString & filename = ":/schema.sql")
{
    M migrator;
    return update(&migrator, filename);
}

}}}    // namespace
