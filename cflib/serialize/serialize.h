/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
		virtual ~Class() = default; \
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
