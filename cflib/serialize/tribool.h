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

#include <cflib/serialize/serialize.h>

namespace cflib { namespace serialize {

class TriBool
{
public:
	TriBool() : state(0) {}
	TriBool(bool state) : state(state ? 1 : 2) {}

	operator bool() const { return state == 1; }
	bool isNull() const { return state == 0; }

	quint8 toInt() const { return state; }
	static TriBool fromInt(quint8 state) { TriBool b; b.state = state; return b; }

private:
	quint8 state;
};

namespace impl {

class SerializeTypeInfoImpl;
void serializeTypeInfo(SerializeTypeInfoImpl & si, TriBool *);
void serializeBER(const TriBool & val, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &);
void deserializeBER(TriBool & val, const quint8 * data, int len, BERDeserializerBase &);
void serializeJS(const TriBool & val, QByteArray & data, JSSerializerBase &);
void deserializeJS(TriBool & val, const quint8 * data, int len, JSDeserializerBase &);

}

}}	// namespace
