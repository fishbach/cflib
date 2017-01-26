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

#include "kafkaproduceconnection.h"

namespace cflib { namespace net {

KafkaConnector::ProduceConnection::ProduceConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
	KafkaConnection(data),
	impl_(impl)
{
}

void KafkaConnector::ProduceConnection::reply(qint32 correlationId, impl::KafkaRawReader & reader)
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
			qint64 offset;
			reader >> partitionId >> errorCode >> offset;
			impl_.main_.produceResponse(correlationId, (KafkaConnector::ErrorCode)errorCode, offset);
		}
	}
}

void KafkaConnector::ProduceConnection::closed()
{
	impl_.produceConnections_.remove(impl_.produceConnections_.keys(this).value(0));
	impl_.fetchMetaData();
}

}}	// namespace
