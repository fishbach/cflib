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
#include <cflib/serialize/impl/js.h>

namespace cflib { namespace serialize { namespace impl {

class JSSerializerBase
{
	Q_DISABLE_COPY(JSSerializerBase)
public:
	JSSerializerBase(QByteArray & data) :
		data_(data), isFirst_(true), lastWasEmpty_(false)
	{
		data_ += '[';
	}

	~JSSerializerBase()
	{
		// In JavaScript the last komma before ']' in an array is dropped
		if (lastWasEmpty_) data_ += ',';
		data_ += ']';
	}

	template<typename T>
	JSSerializerBase & operator<<(const T & cl)
	{
		if (isFirst_) isFirst_ = false;
		else data_ += ',';
		int old = data_.size();
		serializeJS(cl, data_, *this); // *this is needed for C++ ADL
		lastWasEmpty_ = data_.size() == old;
		return *this;
	}

	JSSerializerBase & operator<<(Placeholder)
	{
		if (isFirst_) isFirst_ = false;
		else data_ += ',';
		lastWasEmpty_ = true;
		return *this;
	}

	bool lastWasEmpty() const { return lastWasEmpty_; }

private:
	QByteArray & data_;
	bool isFirst_;
	bool lastWasEmpty_;
};

class JSDeserializerBase
{
public:
	JSDeserializerBase(const quint8 * data, int len) :
		readPos_(data), bytesAvailable_(len), wasLastAvailable_(false)
	{
		if (bytesAvailable_ < 2 || *readPos_ != (quint8)'[' || readPos_[bytesAvailable_ - 1] != (quint8)']') {
			bytesAvailable_ = 0;
		} else {
			bytesAvailable_ -= 2;
			++readPos_;
		}
	}

	template<typename T>
	JSDeserializerBase & operator>>(T & cl)
	{
		const int pos = findNextKommaInJS(readPos_, bytesAvailable_);
		const int avail = (pos == -1) ? bytesAvailable_ : pos;
		if (avail == 0) {
			cl = T();	// default construct
			wasLastAvailable_ = false;
		} else {
			deserializeJS(cl, readPos_, avail, *this); // *this is needed for C++ ADL
			wasLastAvailable_ = true;
		}
		if (pos == -1) bytesAvailable_ = 0;
		else {
			readPos_        += pos + 1;
			bytesAvailable_ -= pos + 1;
		}
		return *this;
	}

	JSDeserializerBase & operator>>(Placeholder)
	{
		int pos = findNextKommaInJS(readPos_, bytesAvailable_);
		if (pos == -1) bytesAvailable_ = 0;
		else {
			readPos_        += pos + 1;
			bytesAvailable_ -= pos + 1;
		}
		return *this;
	}

	bool isAnyAvailable()   const { return bytesAvailable_ > 0; }
	bool wasLastAvailable() const { return wasLastAvailable_; }

private:
	const quint8 * readPos_;
	int bytesAvailable_;
	bool wasLastAvailable_;
};

}}}	// namespace
