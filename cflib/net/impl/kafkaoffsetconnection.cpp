/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "kafkaoffsetconnection.h"

namespace cflib { namespace net {

KafkaConnector::OffsetConnection::OffsetConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
	KafkaConnection(data),
	impl_(impl),
	ok_(false)
{
}

void KafkaConnector::OffsetConnection::reply(qint32 correlationId, impl::KafkaRawReader & reader)
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
			qint64 timestamp;
			qint64 offset;
			reader >> partitionId >> errorCode >> timestamp >> offset;

			if (errorCode == KafkaConnector::NoError) {
				impl_.main_.offsetResponse(correlationId, offset);
				ok_ = true;
			}
		}
	}
	close();
}

void KafkaConnector::OffsetConnection::closed()
{
	if (!ok_) impl_.fetchMetaData();
}

}}	// namespace
