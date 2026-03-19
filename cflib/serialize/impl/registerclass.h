/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/serialize/impl/serializebaseber.h>
#include <cflib/serialize/serializetypeinfo.h>

namespace cflib::serialize::impl {

class RegisterClassBase
{
public:
    template<typename T>
    static inline void serialize(const SharedPtr<T> & cl, BERSerializerBase & ser)
    {
        registry()[cl->getSerializeTypeInfo().classId]->serialize(cl.get(), ser);
    }

    template<typename T>
    static inline void deserialize(SharedPtr<T> & cl, const uint8 * data, int len)
    {
        uint32 classId;
        {
            BERDeserializerBase ser(data, len);
            if (!ser.isAnyAvailable()) {
                cl.reset();
                return;
            }
            ser >> classId;
        }
        auto it = registry().find(classId);
        const RegisterClassBase * basePtr = (it != registry().end()) ? it->second : nullptr;
        if (!basePtr) {
            cl.reset();
            return;
        }
        BERDeserializerBase ser(data, len);
        cl.reset((T *)basePtr->deserialize(ser));
    }

    template<typename T>
    static inline SharedPtr<T> create(uint32 classId)
    {
        SharedPtr<T> rv;
        auto it = registry().find(classId);
        const RegisterClassBase * basePtr = (it != registry().end()) ? it->second : nullptr;
        if (basePtr) rv.reset((T *)basePtr->create());
        return rv;
    }

    static List<SerializeTypeInfo> getAllSerializeTypeInfos()
    {
        List<SerializeTypeInfo> rv;
        for (auto & [id, cl] : registry()) {
            rv.push_back(cl->serializeTypeInfo());
        }
        return rv;
    }

protected:
    static Hash<uint32, const RegisterClassBase *> & registry();
    void duplicateId(uint32 classId);
    virtual void serialize(const void * cl, BERSerializerBase & ser) const = 0;
    virtual void * deserialize(BERDeserializerBase & ser) const = 0;
    virtual SerializeTypeInfo serializeTypeInfo() const = 0;
    virtual void * create() const = 0;
};

template<typename T>
class RegisterClass : public RegisterClassBase
{
public:
    RegisterClass()
    {
        if (registry().count(T::serializeTypeInfo().classId) > 0) duplicateId(T::serializeTypeInfo().classId);
        registry()[T::serializeTypeInfo().classId] = this;
    }

    void serialize(const void * cl, BERSerializerBase & ser) const override
    {
        ((const T *)cl)->serialize(ser);
    }

    void * deserialize(BERDeserializerBase & ser) const override
    {
        T * cl = new T();
        cl->deserialize(ser);
        return cl;
    }

    SerializeTypeInfo serializeTypeInfo() const override
    {
        return T::serializeTypeInfo();
    }

    void * create() const override
    {
        return new T();
    }
};

} // namespace
