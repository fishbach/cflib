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

#include <cflib/serialize/impl/serializejsimpl.h>

namespace cflib { namespace serialize {

class JSSerializer
{
public:
	JSSerializer() : data_(), base_(data_) {}

	JSSerializer(const JSSerializer & other) : data_(other.data_), base_(data_)
	{
		data_.resize(data_.size() - 1);	// remove superfluous '['
	}
	JSSerializer & operator=(const JSSerializer & other) { data_ = other.data_; return *this; }

	QByteArray data() const { return data_ + (base_.lastWasEmpty() ? ",]" : "]"); }

	template<typename T>
	inline JSSerializer & operator<<(const T & cl) { base_ << cl; return *this; }

	inline JSSerializer & operator<<(Placeholder ph) { base_ << ph; return *this; }

private:
	QByteArray data_;
	impl::JSSerializerBase base_;
};

class JSDeserializer
{
public:
	JSDeserializer(const QByteArray & data) :
		data_(data), base_((const quint8 *)data_.constData(), data_.size()) {}

	template<typename T>
	inline JSDeserializer & operator>>(T & cl) { base_ >> cl; return *this; }

	inline JSDeserializer & operator>>(Placeholder ph) { base_ >> ph; return *this; }

	template<typename T>
	inline T get() { T retval; base_ >> retval; return retval; }

private:
	const QByteArray data_;
	impl::JSDeserializerBase base_;
};

}}	// namespace
