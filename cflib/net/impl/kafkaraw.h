/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

	inline uchar * getCurrentRawData()
	{
		return (uchar *)data_.constData();
	}

	inline int getCurrentSize()
	{
		return data_.size();
	}

	inline QByteArray getRawContent()
	{
		return data_.mid(4);
	}

private:
	QByteArray data_;

	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, qint8               val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, qint16              val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, qint32              val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, qint64              val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, const QByteArray  & val);
	friend KafkaRawWriter & operator<<(KafkaRawWriter & out, const KafkaString & val);
};

#define DEFINE_KAFKA_WRITE_INT_OPERATOR(type) \
	inline KafkaRawWriter & operator<<(KafkaRawWriter & out, type val) \
	{ \
		const int oldSize = out.data_.size(); \
		out.data_.resize(oldSize + sizeof(type)); \
		qToBigEndian<type>(val, (uchar *)(out.data_.constData() + oldSize)); \
		return out; \
	} \

DEFINE_KAFKA_WRITE_INT_OPERATOR(qint8 )
DEFINE_KAFKA_WRITE_INT_OPERATOR(qint16)
DEFINE_KAFKA_WRITE_INT_OPERATOR(qint32)
DEFINE_KAFKA_WRITE_INT_OPERATOR(qint64)

inline KafkaRawWriter & operator<<(KafkaRawWriter & out, const QByteArray & val)
{
	if (val.isNull()) {
		out << (qint32)-1;
	} else {
		out << (qint32)val.size();
		out.data_.append(val);
	}
	return out;
}

inline KafkaRawWriter & operator<<(KafkaRawWriter & out, const KafkaString & val)
{
	if (val.isNull()) {
		out << (qint16)-1;
	} else {
		out << (qint16)val.size();
		out.data_.append(val);
	}
	return out;
}


class KafkaRawReader
{
public:
	KafkaRawReader(const QByteArray & data) :
		readPtr_(data.constData()),
		bytesLeft_(data.size())
	{
	}

	inline quint32 bytesLeft() { return bytesLeft_; }

private:
	const char * readPtr_;
	quint32 bytesLeft_;

	friend KafkaRawReader & operator>>(KafkaRawReader & in, qint8       & val);
	friend KafkaRawReader & operator>>(KafkaRawReader & in, qint16      & val);
	friend KafkaRawReader & operator>>(KafkaRawReader & in, qint32      & val);
	friend KafkaRawReader & operator>>(KafkaRawReader & in, qint64      & val);
	friend KafkaRawReader & operator>>(KafkaRawReader & in, QByteArray  & val);
	friend KafkaRawReader & operator>>(KafkaRawReader & in, KafkaString & val);
};

#define DEFINE_KAFKA_READ_INT_OPERATOR(type) \
	inline KafkaRawReader & operator>>(KafkaRawReader & in, type & val) \
	{ \
		if (sizeof(type) > in.bytesLeft_) { \
			val = 0; \
			in.bytesLeft_ = 0; \
		} else { \
			val = qFromBigEndian<type>((const uchar *)in.readPtr_); \
			in.readPtr_   += sizeof(type); \
			in.bytesLeft_ -= sizeof(type); \
		} \
		return in; \
	} \

DEFINE_KAFKA_READ_INT_OPERATOR(qint8 )
DEFINE_KAFKA_READ_INT_OPERATOR(qint16)
DEFINE_KAFKA_READ_INT_OPERATOR(qint32)
DEFINE_KAFKA_READ_INT_OPERATOR(qint64)

inline KafkaRawReader & operator>>(KafkaRawReader & in, QByteArray & val)
{
	qint32 size;
	in >> size;
	if (size < 0) {
		val = QByteArray();
	} else {
		if ((quint32)size > in.bytesLeft_) size = in.bytesLeft_;
		val = QByteArray(in.readPtr_, size);
		in.readPtr_   += size;
		in.bytesLeft_ -= size;
	}
	return in;
}

inline KafkaRawReader & operator>>(KafkaRawReader & in, KafkaString & val)
{
	qint16 size;
	in >> size;
	if (size < 0) {
		val = KafkaString();
	} else {
		if ((quint32)size > in.bytesLeft_) size = in.bytesLeft_;
		val = KafkaString(in.readPtr_, size);
		in.readPtr_   += size;
		in.bytesLeft_ -= size;
	}
	return in;
}

}}}	// namespace
