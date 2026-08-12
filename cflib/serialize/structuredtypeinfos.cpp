/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "structuredtypeinfos.h"

#include <cflib/serialize/impl/registerclass.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib::serialize {

StructuredTypeInfos::StructuredTypeInfos()
{
    for (const SerializeTypeInfo & ti : impl::RegisterClassBase::getAllSerializeTypeInfos()) {
        types_[ti.getName()] = ti;
    }
}

StructuredTypeInfos & StructuredTypeInfos::operator<<(const SerializeTypeInfo & ti)
{
    if (ti.isRMIService()) {
        if (services_.contains(ti.getName())) return *this;
        services_[ti.getName()] = ti;
    } else if (ti.type == SerializeTypeInfo::Class) {
        if (types_.contains(ti.getName())) return *this;
        types_[ti.getName()] = ti;
    } else if (ti.type == SerializeTypeInfo::Using) {
        if (usings_.contains(ti.getName())) return *this;
        usings_[ti.getName()] = ti;
    }
    return *this;
}

SerializeTypeInfos StructuredTypeInfos::fixPlaceholders()
{
    Map<String, SerializeTypeInfo> rv;
    for (auto & [name, ti] : services_) fixPlaceholders(rv, ti);
    return rv.values();
}

SerializeTypeInfos StructuredTypeInfos::types() const
{
    Map<String, SerializeTypeInfo> rv;
    for (const SerializeTypeInfo & ti : services_.values()) checkNeeds(rv, ti);
    return rv.values();
}

void StructuredTypeInfos::checkNeeds(Map<String, SerializeTypeInfo> & needed, const SerializeTypeInfo & ti) const
{
    if (ti.isRMIService()) {
        for (const SerializeFunctionTypeInfo & func : ti.functions + ti.cfSignals) {
            checkNeeds(needed, func.returnType);
            for (const SerializeVariableTypeInfo & param : func.parameters) checkNeeds(needed, param.type);
        }
    } else if (ti.type == SerializeTypeInfo::Class) {
        if (needed.contains(ti.getName())) return;
        needed[ti.getName()] = ti;

        for (const SerializeTypeInfo & base : ti.bases) checkNeeds(needed, base);
        for (const SerializeVariableTypeInfo & member : ti.members) checkNeeds(needed, member.type);

        // needed for dynamic members like: List<SharedPtr<Geometric>> objects;
        // with e.g.: class Circle : public Geometric
        for (const SerializeTypeInfo & derivedClass : types_.values()) {
            if (derivedClass.isDerivedFrom(ti)) checkNeeds(needed, derivedClass);
        }
    } else if (ti.type == SerializeTypeInfo::Container) {
        for (const SerializeTypeInfo & base : ti.bases) checkNeeds(needed, base);
    }
}

void StructuredTypeInfos::fixPlaceholders(Map<String, SerializeTypeInfo> & missing, SerializeTypeInfo & ti)
{
    if (ti.isRMIService()) {
        for (SerializeFunctionTypeInfo & func : ti.functions) {
            fixPlaceholders(missing, func.returnType);
            for (SerializeVariableTypeInfo & param : func.parameters) fixPlaceholders(missing, param.type);
        }
        for (SerializeFunctionTypeInfo & func : ti.cfSignals) {
            fixPlaceholders(missing, func.returnType);
            for (SerializeVariableTypeInfo & param : func.parameters) fixPlaceholders(missing, param.type);
        }
    } else if (ti.type == SerializeTypeInfo::Class) {
        for (SerializeTypeInfo & base : ti.bases) fixPlaceholders(missing, base);
        for (SerializeVariableTypeInfo & member : ti.members) fixPlaceholders(missing, member.type);

        // needed for dynamic members like: List<SharedPtr<Geometric>> objects;
        // with e.g.: class Circle : public Geometric
        for (auto & [name, derivedClass] :  types_) {
            if (derivedClass.isDerivedFrom(ti)) fixPlaceholders(missing, derivedClass);
        }
    } else if (ti.type == SerializeTypeInfo::Container) {
        for (SerializeTypeInfo & base : ti.bases) fixPlaceholders(missing, base);
    } else if (ti.type == SerializeTypeInfo::Placeholder) {
        fixPlaceholder(missing, ti);
    }
}

void StructuredTypeInfos::fixPlaceholder(Map<String, SerializeTypeInfo> & missing, SerializeTypeInfo & ti)
{
    const String phTypeName = ti.typeName;

    static const Regex basicRE(R"(^(bool|u?int(?:8|16|32|64)?|float|(?:long)?double|ByteArray|String|DateTime)$)");
    Regex::MatchResult m = basicRE.matchResult(ti.typeName);
    if (m.hasMatch()) {
        ti.type = SerializeTypeInfo::Basic;
        ti.ns.clear();
        if      (m.captured(0) == "int"       ) ti.typeName = "int32";
        else if (m.captured(0) == "uint"      ) ti.typeName = "uint32";
        else if (m.captured(0) == "float"     ) ti.typeName = "float32";
        else if (m.captured(0) == "double"    ) ti.typeName = "float64";
        else if (m.captured(0) == "longdouble") ti.typeName = "float128";
        else                                    ti.typeName = m.captured(0);
        logDebug("placeholder basic: %1 -> %2", phTypeName, ti.toString());
        return;
    }

    static const Regex container1RE(R"(^(Flags|List|Set)<([^>]+)>$)");
    m = container1RE.matchResult(ti.typeName);
    if (m.hasMatch()) {
        if (m.captured(1) == "Flags") {
            ti.type = SerializeTypeInfo::Basic;
            ti.ns.clear();
            ti.typeName = "int64";
            return;
        }
        ti.type = SerializeTypeInfo::Container;
        SerializeTypeInfo base;
        base.type = SerializeTypeInfo::Placeholder;
        base.ns   = ti.ns;
        base.typeName = m.captured(2);
        ti.bases << base;
        ti.ns.clear();
        fixPlaceholders(missing, ti.bases[0]);
        ti.typeName = "List";
        logDebug("placeholder list: %1 -> %2", phTypeName, ti.toString());
        return;
    }

    static const Regex container2RE(R"(^(Pair|Map|Hash)<([^,]+),([^>]+)>$)");
    m = container2RE.matchResult(ti.typeName);
    if (m.hasMatch()) {
        ti.type = SerializeTypeInfo::Container;
        SerializeTypeInfo base;
        base.type = SerializeTypeInfo::Placeholder;
        base.ns   = ti.ns;
        base.typeName = m.captured(2);
        ti.bases << base;
        base.typeName = m.captured(3);
        ti.bases << base;
        ti.ns.clear();
        fixPlaceholders(missing, ti.bases[0]);
        fixPlaceholders(missing, ti.bases[1]);
        ti.typeName = m.captured(1);
        if (ti.typeName == "Hash") ti.typeName = "Map";
        logDebug("placeholder map: %1 -> %2", phTypeName, ti.toString());
        return;
    }

    String ns = ti.ns;
    forever {
        String name = ns.isEmpty() ? ti.typeName : ns + "::" + ti.typeName;
        if (types_.contains(name)) {
            SerializeTypeInfo & existing = types_[name];
            fixPlaceholders(missing, existing);
            ti = existing;
            logDebug("placeholder obj: %1 -> %2", phTypeName, ti.toString());
            return;
        }
        if (usings_.contains(name)) {
            SerializeTypeInfo & existing = usings_[name].bases[0];
            fixPlaceholders(missing, existing);
            ti = existing;
            logDebug("placeholder obj with using: %1 -> %2", phTypeName, ti.toString());
            return;
        }
        if (ns.isEmpty()) {
            missing[ti.getName()] = ti;
            return;
        }
        ssize_t pos = ns.lastIndexOf("::");
        if (pos == -1) ns.clear();
        else           ns = ns.left(pos);
    }
}

} // namespace
