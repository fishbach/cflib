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

}}}	// namespace
