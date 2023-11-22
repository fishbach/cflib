/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/impl/serializebaseber.h>
#include <cflib/serialize/serializetypeinfo.h>

namespace cflib { namespace serialize { namespace impl {

class RegisterClassBase
{
public:
    template<typename T>
    static inline void serialize(const QSharedPointer<T> & cl, BERSerializerBase & ser)
    {
        registry()[cl->getSerializeTypeInfo().classId]->serialize(cl.data(), ser);
    }

    template<typename T>
    static inline void deserialize(QSharedPointer<T> & cl, const quint8 * data, int len)
    {
        quint32 classId;
        {
            BERDeserializerBase ser(data, len);
            if (!ser.isAnyAvailable()) {
                cl.reset();
                return;
            }
            ser >> classId;
        }
        const RegisterClassBase * basePtr = registry().value(classId);
        if (!basePtr) {
            cl.reset();
            return;
        }
        BERDeserializerBase ser(data, len);
        cl.reset((T *)basePtr->deserialize(ser));
    }

    template<typename T>
    static inline QSharedPointer<T> create(quint32 classId)
    {
        QSharedPointer<T> rv;
        const RegisterClassBase * basePtr = registry().value(classId);
        if (basePtr) rv.reset((T *)basePtr->create());
        return rv;
    }

    static QSet<SerializeTypeInfo> getAllSerializeTypeInfos()
    {
        QSet<SerializeTypeInfo> rv;
        for (const RegisterClassBase * cl : registry().values()) {
            rv << cl->serializeTypeInfo();
        }
        return rv;
    }

protected:
    static QHash<quint32, const RegisterClassBase *> & registry();
    void duplicateId(quint32 classId);
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
        if (registry().contains(T::serializeTypeInfo().classId)) duplicateId(T::serializeTypeInfo().classId);
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

}}}    // namespace
