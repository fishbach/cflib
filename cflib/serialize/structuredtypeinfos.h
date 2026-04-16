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
    StructuredTypeInfos & operator<<(const SerializeTypeInfo & ti);

    List<SerializeTypeInfo> types()    const { return types_   .values(); }
    List<SerializeTypeInfo> services() const { return services_.values(); }

private:
    Map<String, SerializeTypeInfo> types_;
    Map<String, SerializeTypeInfo> services_;
};

} // namespace
