/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/common.h>
#include <cflib/serialize/impl/ber.h>

namespace cflib { namespace serialize { namespace impl {

class BERSerializerBase
{
	Q_DISABLE_COPY(BERSerializerBase)
public:
	BERSerializerBase(QByteArray & data, bool disableTagNumbering = false) :
		data_(data), tag_(disableTagNumbering ? 0 : 1) {}

	template<typename T>
	BERSerializerBase & operator<<(const T & cl)
	{
		serializeBER(cl, tag_, data_, *this); // *this is needed for C++ ADL
		if (tag_ != 0) ++tag_;
		return *this;
	}

	BERSerializerBase & operator<<(Placeholder)
	{
		if (tag_ != 0) ++tag_;
		return *this;
	}

private:
	QByteArray & data_;
	quint64 tag_;
};

class BERDeserializerBase
{
public:
	BERDeserializerBase(const quint8 * data, int len, bool disableTagNumbering = false) :
		readPos_(data), bytesAvailable_(len), tag_(disableTagNumbering ? 0 : 1) {}

	template<typename T>
	BERDeserializerBase & operator>>(T & cl)
	{
		forever {
			// read tlv
			quint64 tag;
			int tagLen;
			int lengthSize;
			const int valueLen = decodeTLV(readPos_, bytesAvailable_, tag, tagLen, lengthSize);
			const int tlvLen = tagLen + lengthSize + valueLen;
			if (valueLen < 0 || bytesAvailable_ < tlvLen) break;

			// check tag and deserialize
			if (tag == tag_) {
				if (valueLen == 0 && lengthSize == 2) {
					cl = T();	// default construct
				} else {
					deserializeBER(cl, readPos_ + tagLen + lengthSize, valueLen, *this); // *this is needed for C++ ADL
				}
				readPos_        += tlvLen;
				bytesAvailable_ -= tlvLen;
				if (tag_ != 0) ++tag_;
				return *this;
			} else if (tag > tag_ && tag_ != 0) {
				cl = T();	// default construct
				if (tag_ != 0) ++tag_;
				return *this;
			}

			// tag < tag_ (placeholder)
			readPos_        += tlvLen;
			bytesAvailable_ -= tlvLen;
		}

		// some error occured
		bytesAvailable_ = 0;
		cl = T();	// default construct
		return *this;
	}

	BERDeserializerBase & operator>>(Placeholder)
	{
		if (tag_ != 0) ++tag_;
		return *this;
	}

	inline bool isAnyAvailable() const { return bytesAvailable_ > 0; }

private:
	const quint8 * readPos_;
	int bytesAvailable_;
	quint64 tag_;
};

}}}	// namespace
