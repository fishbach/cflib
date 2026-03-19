/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "kafkaoffsetconnection.h"

namespace cflib::net {

KafkaConnector::OffsetConnection::OffsetConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
    KafkaConnection(data),
    impl_(impl),
    ok_(false)
{
}

void KafkaConnector::OffsetConnection::reply(int32 correlationId, impl::KafkaRawReader & reader)
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
            int64 timestamp;
            int64 offset;
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

} // namespace
