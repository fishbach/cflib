/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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

#include <cflib/serialize/serializetypeinfo.h>
#include <cflib/serialize/impl/serializeberimpl.h>
#include <cflib/serialize/impl/serializejsimpl.h>

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
	template<> inline void serializeJS(const Class & cl, QByteArray & data, JSSerializerBase &) { \
		if (cl.isNull()) return; \
		JSSerializerBase ser(data); \
		cl.serialize(ser); \
	} \
	}}} \

#define SERIALIZE_CLASS \
	public: \
		template<typename T> void serialize(T & ser) const; \
		template<typename T> void deserialize(T & ser); \
		static cflib::serialize::SerializeTypeInfo serializeTypeInfo(); \

#define serialized
#define rmi \
	public: \
		virtual cflib::serialize::SerializeTypeInfo getServiceInfo() { return serializeTypeInfo(); } \
		virtual void processRMIServiceCallImpl(cflib::serialize::BERDeserializer & deser, uint callNo); \
		virtual void processRMIServiceCallImpl(cflib::serialize::BERDeserializer & deser, uint callNo, cflib::serialize::BERSerializer & ser); \
	public
#define SERIALIZE_BASE
#define SERIALIZE_SKIP(member)
