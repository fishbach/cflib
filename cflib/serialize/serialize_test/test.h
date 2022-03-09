/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serialize.h>

class Test1
{
public:
	int a;
	int b;

	bool operator!=(const Test1 & rhs) const { return a != rhs.a || b != rhs.b; }
	bool isNull() const { return a == 0 && b == 0; }

	template<typename T>
	void serialize(T & ser) const
	{
		ser << a << b;
	}
	template<typename T>
	void deserialize(T & ser)
	{
		ser >> a >> b;
	}
};

SERIALIZE_CLASS_USE_NULL(Test1)

inline QTextStream & operator<<(QTextStream & s, const Test1 & t1)
{
	return s << t1.a << ", " << t1.b;
}

class Test2
{
public:
	Test1 t1;
	int a;

	bool operator!=(const Test2 & rhs) const { return t1 != rhs.t1 || a != rhs.a; }

	template<typename T>
	void serialize(T & ser) const
	{
		ser << t1 << a;
	}
	template<typename T>
	void deserialize(T & ser)
	{
		ser >> t1 >> a;
	}
};

inline QTextStream & operator<<(QTextStream & s, const Test2 & t2)
{
	return s << t2.t1 << ", " << t2.a;
}

template<typename T>
inline QTextStream & operator<<(QTextStream & s, const QList<T> & list)
{
	bool isFirst = true;
	foreach (T el, list) {
		if (isFirst) isFirst = false;
		else         s << ", ";
		s << el;
	}
	return s;
}

template<typename Key, typename Val>
inline QTextStream & operator<<(QTextStream & s, const QMap<Key, Val> & map)
{
	bool isFirst = true;
	foreach (Key key, map.keys()) {
		if (isFirst) isFirst = false;
		else         s << ", ";
		s << key << ": " << map[key];
	}
	return s;
}

template<typename Key, typename Val>
inline QTextStream & operator<<(QTextStream & s, const QHash<Key, Val> & map)
{
	bool isFirst = true;
	foreach (Key key, map.keys()) {
		if (isFirst) isFirst = false;
		else         s << ", ";
		s << key << ": " << map[key];
	}
	return s;
}
