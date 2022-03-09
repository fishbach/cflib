/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
