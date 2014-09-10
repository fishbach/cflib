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

#include <cflib/serialize/common.h>
#include <cflib/serialize/impl/ber.h>

namespace cflib { namespace serialize { namespace impl {

class BERSerializerBase
{
public:
	BERSerializerBase(QByteArray & data, bool disableTagNumbering = false) :
		data_(data), tag_(disableTagNumbering ? 0xC0 : 0xC1), tagLen_(1) {}

	template <typename T>
	BERSerializerBase & operator<<(const T & cl)
	{
		serializeBER(cl, tag_, tagLen_, data_, *this); // *this is needed for C++ ADL
		incTag(tag_, tagLen_);
		return *this;
	}

	BERSerializerBase & operator<<(Placeholder)
	{
		incTag(tag_, tagLen_);
		return *this;
	}

private:
	QByteArray & data_;
	quint64 tag_;
	quint8 tagLen_;
};

class BERDeserializerBase
{
public:
	BERDeserializerBase(const quint8 * data, int len, bool disableTagNumbering = false) :
		readPos_(data), bytesAvailable_(len), tag_(disableTagNumbering ? 0 : 1), tagLen_(1),
		wasLastAvailable_(false) {}

	template <typename T>
	BERDeserializerBase & operator>>(T & cl)
	{
		forever {
			// read tag
			if (bytesAvailable_ < 1) break;
			quint64 tag = *readPos_ & 0x1F;	// remove Class and P/C
			quint8 tagLen = 1;
			if (tag == 0x1F) {
				const quint8 * p = readPos_;
				++tagLen;
				if (bytesAvailable_ < tagLen) break;
				tag <<= 8;
				tag |= *(++p);
				while (tag & 0x80) {
					++tagLen;
					if (bytesAvailable_ < tagLen) break;
					tag <<= 8;
					tag |= *(++p);
				}
			}

			// read length
			int lengthSize;
			const int valueLen = decodeBERLength(readPos_ + tagLen, bytesAvailable_ - tagLen, lengthSize);
			const int tlvLen = tagLen + lengthSize + valueLen;

			// check for broken TLV
			if (valueLen < 0 || bytesAvailable_ < tlvLen) break;

			// check tag and deserialize
			if (tag == tag_) {
				if (valueLen == 0 && lengthSize == 1) {
					cl = T();	// default construct
				} else {
					deserializeBER(cl, readPos_ + tagLen + lengthSize, valueLen, *this); // *this is needed for C++ ADL
				}
				readPos_        += tlvLen;
				bytesAvailable_ -= tlvLen;
				wasLastAvailable_ = true;
				incTag(tag_, tagLen_);
				return *this;
			} else if (tag > tag_) {
				wasLastAvailable_ = false;
				cl = T();	// default construct
				incTag(tag_, tagLen_);
				return *this;
			}

			// tag < tag_
			readPos_        += tlvLen;
			bytesAvailable_ -= tlvLen;
		}

		// some error occured
		bytesAvailable_ = 0;
		wasLastAvailable_ = false;
		cl = T();	// default construct
		return *this;
	}

	BERDeserializerBase & operator>>(Placeholder)
	{
		incTag(tag_, tagLen_);
		return *this;
	}

	bool isAnyAvailable()   const { return bytesAvailable_ > 0; }
	bool wasLastAvailable() const { return wasLastAvailable_; }

private:
	const quint8 * readPos_;
	int bytesAvailable_;
	quint64 tag_;
	quint8 tagLen_;
	bool wasLastAvailable_;
};

}}}	// namespace
