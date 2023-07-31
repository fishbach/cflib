/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
	quint32 classId;
	QString ns;
	QString typeName;
	QList<SerializeTypeInfo> bases;
	QList<SerializeVariableTypeInfo> members;
	QList<SerializeFunctionTypeInfo> functions;
	QList<SerializeFunctionTypeInfo> cfSignals;

public:
	SerializeTypeInfo() : type(Null), classId(0) {}
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
	QList<SerializeVariableTypeInfo> registerParameters;

public:
	QString toString() const;
	QString signature(bool withParamNames = false) const;
	bool hasReturnValues() const { return returnValueCount() > 0; }
	uint returnValueCount() const;
};


}}	// namespace
