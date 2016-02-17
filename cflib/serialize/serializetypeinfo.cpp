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

#include "serializetypeinfo.h"

namespace cflib { namespace serialize {

QString SerializeTypeInfo::toString() const
{
	if (type == Basic) return typeName;

	if (type == Container) {
		QString retval = typeName.left(typeName.indexOf('<') + 1);
		retval += bases[0].toString();
		if (bases.size() > 1) retval += ',' + bases[1].toString();
		retval += '>';
		return retval;
	}

	if (type == Class) {
		QString retval;
		if (!ns.isEmpty()) retval += ns + "::";
		retval += typeName;
		if (!bases.isEmpty()) retval += '[' + bases[0].toString() + ']';
		retval += '{';
		bool isFirst = true;
		foreach (const SerializeVariableTypeInfo & inf, members) {
			if (isFirst) isFirst = false;
			else retval += ',';
			if (inf.type.type != Null) retval += inf.type.toString() + ' ' + inf.name;
		}
		foreach (const SerializeFunctionTypeInfo & fn, functions) {
			if (isFirst) isFirst = false;
			else retval += ',';
			retval += fn.toString();
		}
		foreach (const SerializeFunctionTypeInfo & fn, cfSignals) {
			if (isFirst) isFirst = false;
			else retval += ',';
			retval += fn.toString();
		}
		return retval + '}';
	}

	return "void";
}

QString SerializeTypeInfo::getName() const
{
	if (ns.isEmpty()) return typeName;
	QString retval = ns;
	retval += "::";
	retval += typeName;
	return retval;
}

QString SerializeFunctionTypeInfo::toString() const
{
	QString retval = returnType.toString() + ' ' + name + '(';
	bool isFirst2 = true;
	foreach (const SerializeVariableTypeInfo & inf, parameters) {
		if (isFirst2) isFirst2 = false;
		else retval += ',';
		retval += inf.type.toString();
		if (!inf.name.isEmpty()) retval += ' ' + inf.name;
	}
	retval += ')';
	return retval;
}

QString SerializeFunctionTypeInfo::signature(bool withParamNames) const
{
	QString retval = returnType.typeName;
	if (retval.isEmpty()) retval += "void";
	retval += ' ';
	retval += name;
	retval += '(';
	bool isFirst = true;
	foreach (const SerializeVariableTypeInfo & inf, parameters) {
		if (isFirst) isFirst = false;
		else retval += withParamNames ? ", " : ",";
		retval += inf.type.typeName;
		if (inf.isRef) retval += '&';
		if (withParamNames && !inf.name.isEmpty()) retval += " " + inf.name;
	}
	retval += ')';
	return retval;
}

bool SerializeFunctionTypeInfo::hasReturnValues() const
{
	if (returnType.type != SerializeTypeInfo::Null) return true;
	foreach (const SerializeVariableTypeInfo & inf, parameters) {
		if (inf.isRef) return true;
	}
	return false;
}

}}	// namespace
