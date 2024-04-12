/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "tribool.h"

#include <cflib/serialize/impl/serializetypeinfoimpl.h>

namespace cflib { namespace serialize {

namespace impl {

void serializeTypeInfo(SerializeTypeInfoImpl & si, TriBool *)
{
    si.type = SerializeTypeInfo::Basic;
    si.typeName = "tribool";
}

void serializeBER(const TriBool & val, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
    serializeBERInt(val.toInt(), tagNo, data);
}

void deserializeBER(TriBool & val, const quint8 * data, int len, BERDeserializerBase &)
{
    int v;
    deserializeBERInt(v, data, len);
    val = TriBool::fromInt(v);
}

}

}}    // namespace
