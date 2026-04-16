/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "structuredtypeinfos.h"

#include <cflib/serialize/impl/registerclass.h>

namespace cflib::serialize {

StructuredTypeInfos & StructuredTypeInfos::operator<<(const SerializeTypeInfo & ti)
{
    if (!ti.functions.isEmpty() || !ti.cfSignals.isEmpty()) {
        if (services_.contains(ti.getName())) return *this;

        services_[ti.getName()] = ti;
        for (const SerializeFunctionTypeInfo & func : ti.functions + ti.cfSignals) {
            *this << func.returnType;
            for (const SerializeVariableTypeInfo & param : func.parameters) *this << param.type;
        }

    } else {
        if (types_.contains(ti.getName())) return *this;

        if (ti.type == SerializeTypeInfo::Class) {
            types_[ti.getName()] = ti;
            for (const SerializeTypeInfo & base : ti.bases) *this << base;
            for (const SerializeVariableTypeInfo & member : ti.members) *this << member.type;

            // needed for dynamic members like: List<SharedPtr<Geometric>> objects;
            // with e.g.: class Circle : public Geometric
            for (const SerializeTypeInfo & derivedClass : impl::RegisterClassBase::getAllSerializeTypeInfos()) {
                if (derivedClass.isDerivedFrom(ti)) *this << derivedClass;
            }
        } else if (ti.type == SerializeTypeInfo::Container) {
            for (const SerializeTypeInfo & base : ti.bases) *this << base;
        }
    }

    return *this;
}

} // namespace
