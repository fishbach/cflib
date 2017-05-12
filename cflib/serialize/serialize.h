/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include <cflib/serialize/impl/registerclass.h>
#include <cflib/serialize/serializeber.h>
#include <cflib/serialize/serializetypeinfo.h>

#define SERIALIZE_CLASS_USE_NULL(Class) \
	namespace cflib { namespace serialize { namespace impl { \
	template<> inline void serializeBER(const Class & cl, quint64 tagNo, QByteArray & data, \
		BERSerializerBase &) \
	{ \
		if (cl.isNull()) { writeNull(data, tagNo); return; } \
		TLWriter tlw(data, tagNo); \
		BERSerializerBase ser(data); \
		cl.serialize(ser); \
	} \
	}}} \

#define SERIALIZE_CLASS \
	public: \
		template<typename T> void serialize(T & ser) const; \
		template<typename T> void deserialize(T & ser); \
		static cflib::serialize::SerializeTypeInfo serializeTypeInfo(); \

#define SERIALIZE_IS_BASE(Class) \
	public: \
		virtual ~Class() {} \
		virtual cflib::serialize::SerializeTypeInfo getSerializeTypeInfo() const { return serializeTypeInfo(); } \
		static inline QSharedPointer<Class> createByClassId(quint32 classId) { \
			return cflib::serialize::impl::RegisterClassBase::create<Class>(classId); } \

#define SERIALIZE_BASE(Class) \
	private: \
		static const cflib::serialize::impl::RegisterClass<Class> cflib_serialize_impl_registerClass; \
	public: \
		cflib::serialize::SerializeTypeInfo getSerializeTypeInfo() const override { return serializeTypeInfo(); } \

#define SERIALIZE_STDBASE(Class) \
	private: \
		static const cflib::serialize::impl::RegisterClass<Class> cflib_serialize_impl_registerClass; \
	public: \
		virtual cflib::serialize::SerializeTypeInfo getSerializeTypeInfo() const { return serializeTypeInfo(); } \

#define serialized

#define rmi \
	public: \
		cflib::serialize::SerializeTypeInfo getServiceInfo() const override { return serializeTypeInfo(); } \
		void processRMIServiceCallImpl(cflib::serialize::BERDeserializer & deser, uint callNo) override; \
		void processRMIServiceCallImpl(cflib::serialize::BERDeserializer & deser, uint callNo, cflib::serialize::BERSerializer & ser) override; \
		cflib::net::RSigBase * getCfSignal(uint sigNo) override; \
	public

#define SERIALIZE_SKIP(member)
