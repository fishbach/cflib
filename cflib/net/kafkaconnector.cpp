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

void KafkaConnector::connect(const QByteArray & destAddress, quint16 destPort)
{
	connect(QList<KafkaConnector::Address>() << qMakePair(destAddress, destPort));
}

void KafkaConnector::connect(const QList<KafkaConnector::Address> & cluster)
{
	impl_->connect(cluster);
}

void KafkaConnector::produce(const QByteArray & topic, qint32 partitionId, const Messages & messages,
	quint16 requiredAcks, quint32 ackTimeoutMs, quint32 correlationId)
{
	impl_->produce(topic, partitionId, messages, requiredAcks, ackTimeoutMs, correlationId);
}

void KafkaConnector::getFirstOffset(const QByteArray & topic, qint32 partitionId, quint32 correlationId)
{
	impl_->getOffsets(topic, partitionId, correlationId, true);
}

void KafkaConnector::getHighwaterMarkOffset(const QByteArray & topic, qint32 partitionId, quint32 correlationId)
{
	impl_->getOffsets(topic, partitionId, correlationId, false);
}

void KafkaConnector::fetch(const QByteArray & topic, qint32 partitionId, qint64 offset,
	quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes, quint32 correlationId)
{
	impl_->fetch(topic, partitionId, offset, maxWaitTime, minBytes, maxBytes, correlationId);
}

void KafkaConnector::joinGroup(const QByteArray & groupId, const Topics & topics, GroupAssignmentStrategy preferredStrategy)
{
	impl_->joinGroup(groupId, topics, preferredStrategy);
}

void KafkaConnector::fetch(quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes)
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

}}	// namespace
