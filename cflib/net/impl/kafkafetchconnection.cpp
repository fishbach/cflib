/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include "kafkafetchconnection.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

KafkaConnector::FetchConnection::FetchConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
	KafkaConnection(data),
	impl_(impl)
{
}

void KafkaConnector::FetchConnection::reply(qint32 correlationId, impl::KafkaRawReader & reader)
{
	qint32 topicCount;
	reader >> topicCount;
	for (qint32 i = 0 ; i < topicCount ; ++i) {

		impl::KafkaString topicName;
		reader >> topicName;

		qint32 partitionCount;
		reader >> partitionCount;
		for (qint32 i = 0 ; i < partitionCount ; ++i) {
			qint32 partitionId;
			qint16 errorCode;
			qint64 highwaterMarkOffset;
			qint32 messageSetSize;
			reader >> partitionId >> errorCode >> highwaterMarkOffset >> messageSetSize;

			KafkaConnector::Messages messages;
			qint64 firstOffset = -1;
			bool isFirst = true;
			while (messageSetSize >= 12) {
				const quint32 startPos = reader.bytesLeft();

				qint64 offset;
				qint32 messageSize;
				reader >> offset >> messageSize;
				if (messageSize + 12 > messageSetSize) break;

				if (isFirst) {
					isFirst = false;
					firstOffset = offset;
				}

				qint32 crc;
				qint8 magicByte;
				qint8 attributes;
				KafkaConnector::Message msg;
				reader >> crc >> magicByte >> attributes >> msg.first >> msg.second;
				messages << msg;

				messageSetSize -= startPos - reader.bytesLeft();
			}

			impl_.main_.fetchResponse(correlationId, messages, firstOffset, highwaterMarkOffset, (KafkaConnector::ErrorCode)errorCode);
		}
	}
}

void KafkaConnector::FetchConnection::closed()
{
	impl_.fetchConnections_.remove(impl_.fetchConnections_.keys(this).value(0));
	impl_.fetchMetaData();
}

}}	// namespace
