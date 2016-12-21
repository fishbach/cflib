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

#include <QtCore>

namespace cflib { namespace net { namespace impl {

class KafkaString : public QByteArray
{
public:
	inline KafkaString() {}
	inline KafkaString(const char * data, int size = -1) : QByteArray(data, size) {}
	inline KafkaString(int size, char c) : QByteArray(size, c) {}
	inline KafkaString(const QByteArray & other) : QByteArray(other) {}
};

class KafkaRawWriter
{
public:
	KafkaRawWriter(quint32 expectedSize = 0)
	{
		data_.reserve(4 + expectedSize);
		data_.resize(4);
	}

	inline void reset()
	{
		data_.resize(4);
	}

	inline QByteArray getData()
	{
		qToBigEndian<qint32>(data_.size() - 4, (uchar *)data_.constData());
		return data_;
	}

	inline void expectMoreBytes(quint32 count)
	{
		data_.reserve(data_.size() + count);
	}

private:
	QByteArray data_;

	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, qint8 val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, qint16 val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, qint32 val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, qint64 val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, const QByteArray & val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, const KafkaString & val);
};

#define DEFINE_KAFKA_INT_OPERATOR(type) \
	inline KafkaRawWriter & operator<<(KafkaRawWriter & out, type val) \
	{ \
		const int oldSize = out.data_.size(); \
		out.data_.resize(oldSize + sizeof(val)); \
		qToBigEndian<type>(val, (uchar *)(out.data_.constData() + oldSize)); \
		return out; \
	} \

DEFINE_KAFKA_INT_OPERATOR(qint8)
DEFINE_KAFKA_INT_OPERATOR(qint16)
DEFINE_KAFKA_INT_OPERATOR(qint32)
DEFINE_KAFKA_INT_OPERATOR(qint64)

inline KafkaRawWriter & operator<<(KafkaRawWriter & out, const QByteArray & val)
{
	out << (qint32)val.size();
	out.data_.append(val);
	return out;
}

inline KafkaRawWriter & operator<<(KafkaRawWriter & out, const KafkaString & val)
{
	out << (qint16)val.size();
	out.data_.append(val);
	return out;
}

class KafkaRequest : public KafkaRawWriter
{
public:
	KafkaRequest(qint16 apiKey, qint16 apiVersion = 0) :
		KafkaRawWriter(kafkaClientName.size() + 10)
	{
		*this
			<< apiKey
			<< apiVersion
			<< (qint32)0	// correlationId
			<< kafkaClientName;
	}

private:
	static const cflib::net::impl::KafkaString kafkaClientName;

};

}}}	// namespace
