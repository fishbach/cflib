/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

class DynamicBase
{
	SERIALIZE_CLASS
	SERIALIZE_IS_BASE(DynamicBase)
public serialized:
	int x;

public:
	DynamicBase() : x(0) {}
};

class DynamicA : public DynamicBase
{
	SERIALIZE_CLASS
	SERIALIZE_BASE(DynamicA)
public serialized:
	int a;

public:
	DynamicA() : a(0) {}
};

class DynamicB : public DynamicBase
{
	SERIALIZE_CLASS
	SERIALIZE_BASE(DynamicB)
public serialized:
	double b;

public:
	DynamicB() : b(0) {}
};

class DynamicUse
{
	SERIALIZE_CLASS
public serialized:
	int y;
	QSharedPointer<DynamicBase> d;
	typedef QSharedPointer<DynamicBase> DBase;
	QList<DBase> e;
	int z;

public:
	DynamicUse() : y(0), z(0) {}
};
