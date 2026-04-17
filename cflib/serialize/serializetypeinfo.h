/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::serialize {

class SerializeVariableTypeInfo;
class SerializeFunctionTypeInfo;

class SerializeTypeInfo
{
public:
    SerializeTypeInfo();
    SerializeTypeInfo(const SerializeTypeInfo & other);
    SerializeTypeInfo(SerializeTypeInfo && other);
    SerializeTypeInfo & operator=(const SerializeTypeInfo & other);
    SerializeTypeInfo & operator=(SerializeTypeInfo && other);
    ~SerializeTypeInfo();

    enum Type {
        Null = 0,
        Basic,
        Class,
        Container
    };

    Type type = Null;
    uint32 classId = 0;
    String ns;
    String typeName;
    List<SerializeTypeInfo> bases;
    List<SerializeVariableTypeInfo> members;
    List<SerializeFunctionTypeInfo> functions;
    List<SerializeFunctionTypeInfo> cfSignals;

public:
    bool operator==(const SerializeTypeInfo & rhs) const { return getName() == rhs.getName(); }
    bool operator<(const SerializeTypeInfo & rhs) const { return getName() < rhs.getName(); }
    String toString() const;
    String getName() const;
    bool isDerivedFrom(const SerializeTypeInfo & base) const;
};

class SerializeVariableTypeInfo
{
public:
    String name;
    SerializeTypeInfo type;
    bool isRef = false;

public:
    SerializeVariableTypeInfo() = default;
    SerializeVariableTypeInfo(const String & name, const SerializeTypeInfo & type, bool isRef = false) :
        name(name), type(type), isRef(isRef) {}
};

class SerializeFunctionTypeInfo
{
public:
    String name;
    SerializeTypeInfo returnType;
    List<SerializeVariableTypeInfo> parameters;
    List<SerializeVariableTypeInfo> registerParameters;

public:
    String toString() const;
    String signature(bool withParamNames = false) const;
    bool hasReturnValues() const { return returnValueCount() > 0; }
    uint returnValueCount() const;
};

} // namespace

namespace std {
template<> struct hash<cflib::serialize::SerializeTypeInfo> {
    size_t operator()(const cflib::serialize::SerializeTypeInfo & s) const {
        return hash<cflib::base::String>()(s.getName());
    }
};
}
