/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serialize.h>

namespace cflib { namespace serialize {

class TriBool
{
public:
    TriBool() : state_(0) {}
    TriBool(bool state) : state_(state ? 1 : 2) {}

    operator bool() const { return state_ == 1; }
    bool isNull() const   { return state_ == 0; }
    bool isFalse() const  { return state_ == 2; }

    bool operator==(const TriBool & rhs) const { return state_ == rhs.state_; }
    bool operator!=(const TriBool & rhs) const { return !operator==(rhs); }

    cfuint8 toInt() const { return state_; }
    static TriBool fromInt(cfuint8 state) { TriBool b; b.state_ = state; return b; }

    template<typename T> void serialize(T & ser) const {
        ser << state_;
    }
    template<typename T> void deserialize(T & ser) {
        ser >> state_;
    }

private:
    cfuint8 state_;
};

namespace impl {

class SerializeTypeInfoImpl;
void serializeTypeInfo(SerializeTypeInfoImpl & si, TriBool *);
void serializeBER(const TriBool & val, cfuint64 tag, CFByteArray & data, BERSerializerBase &);
void deserializeBER(TriBool & val, const cfuint8 * data, int len, BERDeserializerBase &);

}

}}    // namespace
