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

#include <QtCore>

namespace cflib { namespace serialize {

class SerializeVariableTypeInfo;
class SerializeFunctionTypeInfo;

class SerializeTypeInfo
{
public:
	enum Type {
		Null = 0,
		Basic,
		Class,
		Container
	};

	Type type;
	QString ns;
	QString typeName;
	QList<SerializeTypeInfo> bases;
	QList<SerializeVariableTypeInfo> members;
	QList<SerializeFunctionTypeInfo> functions;

public:
	SerializeTypeInfo() : type(Null) {}
	bool operator==(const SerializeTypeInfo & rhs) const { return getName() == rhs.getName(); }
	QString toString() const;
	QString getName() const;
};

inline uint qHash(const SerializeTypeInfo & ti, uint seed = 0)
{
	return qHash(ti.getName(), seed);
}

class SerializeVariableTypeInfo
{
public:
	QString name;
	SerializeTypeInfo type;
	bool isRef;

public:
	SerializeVariableTypeInfo() : isRef(false) {}
	SerializeVariableTypeInfo(const QString & name, const SerializeTypeInfo & type, bool isRef = false) :
		name(name), type(type), isRef(isRef) {}
};

class SerializeFunctionTypeInfo
{
public:
	QString name;
	SerializeTypeInfo returnType;
	QList<SerializeVariableTypeInfo> parameters;

public:
	QString toString() const;
	QString signature(bool withParamNames = false) const;
	bool hasReturnValues() const;
};

}}	// namespace
