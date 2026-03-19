/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "kafkaproduceconnection.h"

namespace cflib::net {

KafkaConnector::ProduceConnection::ProduceConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
    KafkaConnection(data),
    impl_(impl)
{
}

void KafkaConnector::ProduceConnection::reply(int32 correlationId, impl::KafkaRawReader & reader)
{
    int32 topicCount;
    reader >> topicCount;
    for (int32 i = 0 ; i < topicCount ; ++i) {

        impl::KafkaString topicName;
        reader >> topicName;

        int32 partitionCount;
        reader >> partitionCount;
        for (int32 i = 0 ; i < partitionCount ; ++i) {
            int32 partitionId;
            int16 errorCode;
            int64 offset;
            reader >> partitionId >> errorCode >> offset;
            impl_.main_.produceResponse(correlationId, (KafkaConnector::ErrorCode)errorCode, offset);
        }
    }
}

void KafkaConnector::ProduceConnection::closed()
{
    for (auto it = impl_.produceConnections_.begin(); it != impl_.produceConnections_.end(); ++it) {
        if (it->second == this) { impl_.produceConnections_.erase(it); break; }
    }
    impl_.fetchMetaData();
}

} // namespace
