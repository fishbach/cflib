/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "serializetypeinfo.h"

namespace cflib::serialize {

// needed here because of forward declaration of SerializeFunctionTypeInfo
SerializeTypeInfo::SerializeTypeInfo() = default;
SerializeTypeInfo::SerializeTypeInfo(const SerializeTypeInfo & other) = default;
SerializeTypeInfo::SerializeTypeInfo(SerializeTypeInfo && other) = default;
SerializeTypeInfo & SerializeTypeInfo::operator=(const SerializeTypeInfo & other) = default;
SerializeTypeInfo & SerializeTypeInfo::operator=(SerializeTypeInfo && other) = default;
SerializeTypeInfo::~SerializeTypeInfo() = default;

SerializeTypeInfo SerializeTypeInfo::getBase() const
{
    if (type != Class || bases.isEmpty()) return {};
    return bases.first();
}

String SerializeTypeInfo::toString() const
{
    if (type == Basic) return typeName;

    if (type == Container) {
        String retval = typeName;
        retval += '<';
        retval += bases[0].toString();
        if (bases.size() > 1) retval += String(",") + bases[1].toString();
        retval += '>';
        return retval;
    }

    if (type == Class) {
        String retval;
        if (!ns.isEmpty()) retval += ns + "::";
        retval += typeName;
        if (!bases.isEmpty()) retval += String("[") + bases[0].toString() + "]";
        retval += '{';
        bool isFirst = true;
        for (const SerializeVariableTypeInfo & inf : members) {
            if (isFirst) isFirst = false;
            else retval += ',';
            if (inf.type.type != Null) retval += inf.type.toString() + " " + inf.name;
        }
        for (const SerializeFunctionTypeInfo & fn : functions) {
            if (isFirst) isFirst = false;
            else retval += ',';
            retval += fn.toString();
        }
        for (const SerializeFunctionTypeInfo & fn : cfSignals) {
            if (isFirst) isFirst = false;
            else retval += ',';
            retval += fn.toString();
        }
        return retval + "}";
    }

    if (type == Placeholder) {
        String retval;
        if (!ns.isEmpty()) retval += ns + " :: ";
        retval += typeName;
        retval += " (placeholder)";
        return retval;
    }

    return "void";
}

String SerializeTypeInfo::getName(bool absNs) const
{
    if (type == Container) {
        String retval = typeName;
        retval += '<';
        bool isFirst = true;
        for (const SerializeTypeInfo & base : bases) {
            if (isFirst) isFirst = false;
            else         retval += ',';
            retval += base.getName(absNs);
        }
        retval += '>';
        return retval;
    }

    if (ns.isEmpty()) return typeName;
    String retval;
    if (absNs) retval = "::";
    retval += ns;
    retval += "::";
    retval += typeName;
    return retval;
}

String SerializeTypeInfo::getFilePath() const
{
    return getName().replace("::", "/").toLower();
}

String SerializeTypeInfo::getNSPath() const
{
    return String(ns).replace("::", "/").toLower();
}

bool SerializeTypeInfo::isDerivedFrom(const SerializeTypeInfo & base) const
{
    for (const SerializeTypeInfo & ti : bases) {
        if (ti.getName() == base.getName()) return true;
        if (ti.isDerivedFrom(base)) return true;
    }
    return false;
}

SerializeTypeInfos SerializeTypeInfo::allUsedClasses() const
{
    Set<SerializeTypeInfo> allClasses;
    for (const SerializeTypeInfo         & ti  : bases  ) allClasses << ti;
    for (const SerializeVariableTypeInfo & vti : members) allClasses << vti.type;
    for (const SerializeFunctionTypeInfo & fti : functions) {
        allClasses << fti.returnType;
        for (const SerializeVariableTypeInfo & vti : fti.parameters) allClasses << vti.type;
    }
    for (const SerializeFunctionTypeInfo & fti : cfSignals) {
        allClasses << fti.returnType;
        for (const SerializeVariableTypeInfo & vti : fti.parameters) allClasses << vti.type;
    }

    Set<SerializeTypeInfo> rv;
    for (const SerializeTypeInfo & ti : allClasses) {
        if (ti.type == Class) {
            rv << ti;
        } else if (ti.type == Container) {
            for (const SerializeTypeInfo & ti : ti.bases) rv << ti;
        }
    }
    return rv.toList().sorted();
}

String SerializeFunctionTypeInfo::toString() const
{
    String retval = returnType.toString() + " " + name + "(";
    bool isFirst2 = true;
    for (const SerializeVariableTypeInfo & inf : parameters) {
        if (isFirst2) isFirst2 = false;
        else retval += ',';
        retval += inf.type.toString();
        if (!inf.name.isEmpty()) retval += " " + inf.name;
    }
    retval += ')';
    return retval;
}

String SerializeFunctionTypeInfo::signature(bool withParamNames) const
{
    String retval = returnType.getName();
    if (retval.isEmpty()) retval += "void";
    retval += ' ';
    retval += name;
    retval += '(';
    bool isFirst = true;
    for (const SerializeVariableTypeInfo & inf : parameters) {
        if (isFirst) isFirst = false;
        else retval += withParamNames ? ", " : ",";
        retval += inf.type.getName();
        if (inf.isRef) retval += '&';
        if (withParamNames && !inf.name.isEmpty()) retval += " " + inf.name;
    }
    retval += ')';
    return retval;
}

uint SerializeFunctionTypeInfo::returnValueCount() const
{
    uint rv = 0;
    if (returnType.type != SerializeTypeInfo::Null) ++rv;
    for (const SerializeVariableTypeInfo & inf : parameters) {
        if (inf.isRef) ++rv;
    }
    return rv;
}

} // namespace
