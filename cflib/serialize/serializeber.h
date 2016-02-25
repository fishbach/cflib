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

#include <cflib/serialize/impl/serializeberimpl.h>

namespace cflib { namespace serialize {

class BERSerializer
{
public:
	BERSerializer(quint64 tagNo = 0) : lenPos_(0), data_(), base_(data_) {
		if (tagNo > 0) {
			const quint8 tagLen = impl::calcTagLen(tagNo);
			data_.resize(tagLen + 1);
			quint8 * pos = (quint8 *)data_.constData();
			impl::writeTagBytes(pos, tagNo, true, tagLen);
			pos[tagLen] = '\0';
			lenPos_ = tagLen + 1;
		}
	}

	QByteArray data() {
		if (lenPos_) {
			impl::insertBERLength(data_, lenPos_);
			lenPos_ = 0;
		}
		return data_;
	}

	template<typename T>
	inline BERSerializer & operator<<(const T & cl) { base_ << cl; return *this; }

	inline BERSerializer & operator<<(Placeholder ph) { base_ << ph; return *this; }

private:
	int lenPos_;
	QByteArray data_;
	impl::BERSerializerBase base_;
};

class BERDeserializer
{
public:
	BERDeserializer(const QByteArray & data) :
		data_(data), base_((const quint8 *)data_.constData(), data_.size()) {}
	BERDeserializer(const QByteArray & ba, const quint8 * data, int len) :
		data_(ba), base_(data, len) {}

	template<typename T>
	inline BERDeserializer & operator>>(T & cl) { base_ >> cl; return *this; }

	inline BERDeserializer & operator>>(Placeholder ph) { base_ >> ph; return *this; }

	template<typename T>
	inline T get() { T retval; base_ >> retval; return std::move(retval); }

private:
	const QByteArray data_;
	impl::BERDeserializerBase base_;
};

}}	// namespace
