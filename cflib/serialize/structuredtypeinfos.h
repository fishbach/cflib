/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serializetypeinfo.h>

namespace cflib::serialize {

class StructuredTypeInfos
{
public:
    StructuredTypeInfos();

    StructuredTypeInfos & operator<<(const SerializeTypeInfo & ti);

    // returns missing types
    SerializeTypeInfos fixPlaceholders();

    // Only types needed for API will be returned.
    SerializeTypeInfos types()        const;
    SerializeTypeInfos services()     const { return services_.values(); }

private:
    void checkNeeds(Map<String, SerializeTypeInfo> & needed, const SerializeTypeInfo & ti) const;
    void fixPlaceholders(Map<String, SerializeTypeInfo> & missing, SerializeTypeInfo & ti);
    void fixPlaceholder(Map<String, SerializeTypeInfo> & missing, SerializeTypeInfo & ti);

private:
    Map<String, SerializeTypeInfo> services_;
    Map<String, SerializeTypeInfo> types_;
};

} // namespace
