/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/impl/kafkaconnectorimpl.h>

namespace cflib { namespace net {

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

void KafkaConnector::connect(const CFByteArray & destAddress, cfuint16 destPort)
{
    CFList<KafkaConnector::Address> cluster;
    cluster.push_back(Address(destAddress, destPort));
    connect(cluster);
}

void KafkaConnector::connect(const CFList<KafkaConnector::Address> & cluster)
{
    impl_->connect(cluster);
}

void KafkaConnector::produce(const CFByteArray & topic, cfint32 partitionId, const Messages & messages,
    cfuint16 requiredAcks, cfuint32 ackTimeoutMs, cfuint32 correlationId)
{
    impl_->produce(topic, partitionId, messages, requiredAcks, ackTimeoutMs, correlationId);
}

void KafkaConnector::getFirstOffset(const CFByteArray & topic, cfint32 partitionId, cfuint32 correlationId)
{
    impl_->getOffsets(topic, partitionId, correlationId, true);
}

void KafkaConnector::getHighwaterMarkOffset(const CFByteArray & topic, cfint32 partitionId, cfuint32 correlationId)
{
    impl_->getOffsets(topic, partitionId, correlationId, false);
}

void KafkaConnector::fetch(const CFByteArray & topic, cfint32 partitionId, cfint64 offset,
    cfuint32 maxWaitTime, cfuint32 minBytes, cfuint32 maxBytes, cfuint32 correlationId)
{
    impl_->fetch(topic, partitionId, offset, maxWaitTime, minBytes, maxBytes, correlationId);
}

void KafkaConnector::joinGroup(const CFByteArray & groupId, const Topics & topics, GroupAssignmentStrategy preferredStrategy)
{
    impl_->joinGroup(groupId, topics, preferredStrategy);
}

void KafkaConnector::fetch(cfuint32 maxWaitTime, cfuint32 minBytes, cfuint32 maxBytes)
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

}}    // namespace
