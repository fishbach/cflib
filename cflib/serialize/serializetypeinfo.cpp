/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "serializetypeinfo.h"

namespace cflib { namespace serialize {

CFString SerializeTypeInfo::toString() const
{
    if (type == Basic) return typeName;

    if (type == Container) {
        CFString retval = typeName.left(typeName.indexOf('<') + 1);
        retval += bases[0].toString();
        if (bases.size() > 1) retval += CFString(",") + bases[1].toString();
        retval += '>';
        return retval;
    }

    if (type == Class) {
        CFString retval;
        if (!ns.isEmpty()) retval += ns + "::";
        retval += typeName;
        if (!bases.empty()) retval += CFString("[") + bases[0].toString() + "]";
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

    return "void";
}

CFString SerializeTypeInfo::getName() const
{
    if (ns.isEmpty()) return typeName;
    CFString retval = ns;
    retval += "::";
    retval += typeName;
    return retval;
}

CFString SerializeFunctionTypeInfo::toString() const
{
    CFString retval = returnType.toString() + " " + name + "(";
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

CFString SerializeFunctionTypeInfo::signature(bool withParamNames) const
{
    CFString retval = returnType.typeName;
    if (retval.isEmpty()) retval += "void";
    retval += ' ';
    retval += name;
    retval += '(';
    bool isFirst = true;
    for (const SerializeVariableTypeInfo & inf : parameters) {
        if (isFirst) isFirst = false;
        else retval += withParamNames ? ", " : ",";
        retval += inf.type.typeName;
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

}}    // namespace
