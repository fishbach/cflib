/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "kafkafetchconnection.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Network)

namespace cflib::net {

KafkaConnector::FetchConnection::FetchConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
    KafkaConnection(data),
    impl_(impl)
{
}

void KafkaConnector::FetchConnection::reply(int32 correlationId, impl::KafkaRawReader & reader)
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
            int64 highwaterMarkOffset;
            int32 messageSetSize;
            reader >> partitionId >> errorCode >> highwaterMarkOffset >> messageSetSize;

            KafkaConnector::Messages messages;
            int64 firstOffset = -1;
            bool isFirst = true;
            while (messageSetSize >= 12) {
                const uint32 startPos = reader.bytesLeft();

                int64 offset;
                int32 messageSize;
                reader >> offset >> messageSize;
                if (messageSize + 12 > messageSetSize) break;

                if (isFirst) {
                    isFirst = false;
                    firstOffset = offset;
                }

                int32 crc;
                int8 magicByte;
                int8 attributes;
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
    for (auto it = impl_.fetchConnections_.begin(); it != impl_.fetchConnections_.end(); ++it) {
        if (it->second == this) { impl_.fetchConnections_.erase(it); break; }
    }
    impl_.fetchMetaData();
}

} // namespace
