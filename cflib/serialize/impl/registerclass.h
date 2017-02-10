/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cflib/serialize/impl/serializebaseber.h>

namespace cflib { namespace serialize { namespace impl {

class RegisterClassBase
{
public:
	template<typename T>
	static inline void serialize(const QSharedPointer<T> & cl, BERSerializerBase & ser) {
		registry()[cl->getSerializeTypeInfo().classId]->serialize(cl.data(), ser);
	}

	template<typename T>
	static inline void deserialize(QSharedPointer<T> & cl, const quint8 * data, int len) {
		quint32 classId;
		{
			BERDeserializerBase ser(data, len);
			if (!ser.isAnyAvailable()) {
				cl.reset();
				return;
			}
			ser >> classId;
		}
		BERDeserializerBase ser(data, len);
		cl.reset((T *)registry()[classId]->deserialize(ser));
	}

protected:
	static QHash<quint32, const RegisterClassBase *> & registry();
	void duplicateId(quint32 classId);
	virtual void serialize(const void * cl, BERSerializerBase & ser) const = 0;
	virtual void * deserialize(BERDeserializerBase & ser) const = 0;
};

template<typename T>
class RegisterClass : public RegisterClassBase
{
public:
	RegisterClass() {
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

};

}}}	// namespace
