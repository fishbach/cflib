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

#pragma once

#include <cflib/serialize/serialize.h>

class Dao
{
	SERIALIZE_CLASS
public serialized:
	QString name;
	quint32 number;
	typedef QList<quint32> List;
	typedef QPair<List, QDateTime> Pair;
	QPair<quint8, Pair> pair;
};

class Dao2
{
	SERIALIZE_CLASS
public serialized:
	Dao dao;
	QList<int> numbers;
};

class Dao3 : public Dao2
{
	SERIALIZE_CLASS
	SERIALIZE_BASE
public serialized:
	QDateTime timestamp;
};