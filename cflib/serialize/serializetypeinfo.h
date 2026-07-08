/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::serialize {

class SerializeTypeInfo;
class SerializeVariableTypeInfo;
class SerializeFunctionTypeInfo;

using SerializeTypeInfos         = List<SerializeTypeInfo>;
using SerializeVariableTypeInfos = List<SerializeVariableTypeInfo>;
using SerializeFunctionTypeInfos = List<SerializeFunctionTypeInfo>;

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
        Container,
        Placeholder
    };

    Type type = Null;
    uint32 classId = 0;
    String ns;
    String typeName;
    SerializeTypeInfos bases;
    SerializeVariableTypeInfos members;
    SerializeFunctionTypeInfos functions;
    SerializeFunctionTypeInfos cfSignals;

public:
    bool operator==(const SerializeTypeInfo & rhs) const { return getName() == rhs.getName(); }
    bool operator<(const SerializeTypeInfo & rhs) const { return getName() < rhs.getName(); }

    bool isNull() const { return type == Null; }
    String getName(bool absNs = false) const;
    String getFilePath() const;
    String getNSPath() const;

    bool hasBase() const { return !getBase().isNull(); }
    SerializeTypeInfo getBase() const;
    bool isDerivedFrom(const SerializeTypeInfo & base) const;
    bool isRMIService() const { return !functions.isEmpty() || !cfSignals.isEmpty(); }

    SerializeTypeInfos allUsedClasses() const;

    String toString() const;
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
    SerializeVariableTypeInfos parameters;
    SerializeVariableTypeInfos registerParameters;

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
