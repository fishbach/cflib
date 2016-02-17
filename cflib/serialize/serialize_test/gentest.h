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

#include <cflib/net/rsig.h>
#include <cflib/serialize/serialize.h>
#include <cflib/serialize/serializeber.h>

class GenTest1
{
	SERIALIZE_CLASS
public:
	void f1(int) {}
	int f2() { return 4; }

rmi:
	void f3(int, QString) {}
	QList<int> f4() { return QList<int>(); }
	int f5(int x, int y);
	void f6();

public:
	int x;

public serialized:
	int a;
	SERIALIZE_SKIP(int b)
	int c;
	QString d;

public:
	int y;
};

inline int GenTest1::f5(int x, int y)
{
	return x + y;
}

inline void GenTest1::f6()
{
}

class GenTest2
{
	SERIALIZE_CLASS
public serialized:
	GenTest1 a;
	int b;
};

namespace gentest {

class GenTest3 : public GenTest1
{
	SERIALIZE_CLASS
	SERIALIZE_BASE
public serialized:
	int e;

	class Inner1 {
		SERIALIZE_CLASS
	public serialized:
		int a;
	};
	struct Inner2 {
		SERIALIZE_CLASS
	};
	struct Inner3 {};

	int f;

private:
	int z2;

public:
	int func() const { return z2; }
};

namespace gentest2 {

class GenTest4 : public QList<QString>
{
	SERIALIZE_CLASS
	SERIALIZE_BASE
public serialized:
	int a;
	QList<int> b;
	QList<GenTest2> c;
};

}

}

class GenTest5
{
public serialized:
	int a;
};
