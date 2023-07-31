/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
