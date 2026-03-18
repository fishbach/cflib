/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

void KafkaConnector::FetchConnection::reply(cfint32 correlationId, impl::KafkaRawReader & reader)
{
    cfint32 topicCount;
    reader >> topicCount;
    for (cfint32 i = 0 ; i < topicCount ; ++i) {

        impl::KafkaString topicName;
        reader >> topicName;

        cfint32 partitionCount;
        reader >> partitionCount;
        for (cfint32 i = 0 ; i < partitionCount ; ++i) {
            cfint32 partitionId;
            cfint16 errorCode;
            cfint64 highwaterMarkOffset;
            cfint32 messageSetSize;
            reader >> partitionId >> errorCode >> highwaterMarkOffset >> messageSetSize;

            KafkaConnector::Messages messages;
            cfint64 firstOffset = -1;
            bool isFirst = true;
            while (messageSetSize >= 12) {
                const cfuint32 startPos = reader.bytesLeft();

                cfint64 offset;
                cfint32 messageSize;
                reader >> offset >> messageSize;
                if (messageSize + 12 > messageSetSize) break;

                if (isFirst) {
                    isFirst = false;
                    firstOffset = offset;
                }

                cfint32 crc;
                cfint8 magicByte;
                cfint8 attributes;
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

}}    // namespace
