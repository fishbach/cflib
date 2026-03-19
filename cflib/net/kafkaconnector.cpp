/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/impl/kafkaconnectorimpl.h>

namespace cflib::net {

KafkaConnector::KafkaConnector(util::ThreadVerify * other) :
    impl_(other ?
        new Impl(*this, other) :
        new Impl(*this))
{
}

KafkaConnector::~KafkaConnector()
{
    delete impl_;
}

void KafkaConnector::connect(const ByteArray & destAddress, uint16 destPort)
{
    List<KafkaConnector::Address> cluster;
    cluster.push_back(Address(destAddress, destPort));
    connect(cluster);
}

void KafkaConnector::connect(const List<KafkaConnector::Address> & cluster)
{
    impl_->connect(cluster);
}

void KafkaConnector::produce(const ByteArray & topic, int32 partitionId, const Messages & messages,
    uint16 requiredAcks, uint32 ackTimeoutMs, uint32 correlationId)
{
    impl_->produce(topic, partitionId, messages, requiredAcks, ackTimeoutMs, correlationId);
}

void KafkaConnector::getFirstOffset(const ByteArray & topic, int32 partitionId, uint32 correlationId)
{
    impl_->getOffsets(topic, partitionId, correlationId, true);
}

void KafkaConnector::getHighwaterMarkOffset(const ByteArray & topic, int32 partitionId, uint32 correlationId)
{
    impl_->getOffsets(topic, partitionId, correlationId, false);
}

void KafkaConnector::fetch(const ByteArray & topic, int32 partitionId, int64 offset,
    uint32 maxWaitTime, uint32 minBytes, uint32 maxBytes, uint32 correlationId)
{
    impl_->fetch(topic, partitionId, offset, maxWaitTime, minBytes, maxBytes, correlationId);
}

void KafkaConnector::joinGroup(const ByteArray & groupId, const Topics & topics, GroupAssignmentStrategy preferredStrategy)
{
    impl_->joinGroup(groupId, topics, preferredStrategy);
}

void KafkaConnector::fetch(uint32 maxWaitTime, uint32 minBytes, uint32 maxBytes)
{
    impl_->fetch(maxWaitTime, minBytes, maxBytes);
}

void KafkaConnector::commit()
{
    impl_->commit();
}

void KafkaConnector::leaveGroup()
{
    impl_->leaveGroup();
}

} // namespace
