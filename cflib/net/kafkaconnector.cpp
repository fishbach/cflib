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
#include <cflib/util/log.h>
#include <cflib/util/timer.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Network)

#include <cflib/net/impl/kafkafetch.h>
#include <cflib/net/impl/kafkametadata.h>
#include <cflib/net/impl/kafkaproduce.h>

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

void KafkaConnector::fetch(const QByteArray & topic, qint32 partitionId, qint64 offset,
	quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes, quint32 correlationId)
{
	impl_->fetch(topic, partitionId, offset, maxWaitTime, minBytes, maxBytes, correlationId);
}

void KafkaConnector::joinGroup(const QByteArray & groupName, const QList<QByteArray> & topics)
{
	impl_->joinGroup(groupName, topics);
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

// ============================================================================
// ============================================================================

KafkaConnector::Impl::Impl(KafkaConnector &main) :
	ThreadVerify("KafkaConnector", ThreadVerify::Net),
	main_(main),
	net_(0, this),
	clusterId_(0),
	currentState_(KafkaConnector::Idle)
{
}

KafkaConnector::Impl::Impl(KafkaConnector &parent, util::ThreadVerify *other) :
	ThreadVerify(other),
	main_(parent),
	net_(0, this),
	clusterId_(0),
	currentState_(KafkaConnector::Idle)
{
}

KafkaConnector::Impl::~Impl()
{
	logFunctionTrace
	stopVerifyThread();
}

void KafkaConnector::Impl::setState(KafkaConnector::State newState)
{
	if (currentState_ == newState) return;

	currentState_ = newState;
	main_.stateChanged(newState);
}

void KafkaConnector::Impl::connect(const QList<KafkaConnector::Address> &cluster)
{
	if (!verifyThreadCall(&Impl::connect, cluster)) return;

	if (!cluster_.isEmpty()) {
		/// TODO abort all
	}

	cluster_ = cluster;
	clusterId_ = 0;

	fetchMetaData();
}

void KafkaConnector::Impl::fetchMetaData()
{
	logFunctionTrace

	if (cluster_.isEmpty()) {
		setState(Idle);
		return;
	}

	setState(Connecting);

	const int startId = clusterId_;
	do {
		const KafkaConnector::Address & addr = cluster_[clusterId_];
		if (++clusterId_ >= cluster_.size()) clusterId_ = 0;
		TCPConnData * data = net_.openConnection(addr.first, addr.second);
		if (data) {
			new KafkaConnector::MetadataConnection(data, *this);
			return;
		}
	} while (clusterId_ != startId);

	logWarn("could not connect to kafka cluster");
	util::Timer::singleShot(1.0, this, &Impl::fetchMetaData);
}

void KafkaConnector::Impl::produce(const QByteArray & topic, qint32 partitionId, const Messages & messages,
	quint16 requiredAcks, quint32 ackTimeoutMs, quint32 correlationId)
{
	if (!verifyThreadCall(&Impl::produce, topic, partitionId, messages, requiredAcks, ackTimeoutMs, correlationId)) return;

	// get broker for topic and partition
	qint32 nodeId = responsibilities_[topic][partitionId].id;
	if (nodeId == -1) {
		if (requiredAcks != 0) main_.produceResponse(correlationId, KafkaConnector::UnknownTopicOrPartition, -1);
		fetchMetaData();
		return;
	}

	// get tcp connection of broker
	KafkaConnector::ProduceConnection *& conn = produceConnections_[nodeId];
	if (!conn) {
		const KafkaConnector::Address & addr = allBrokers_[nodeId];
		TCPConnData * data = net_.openConnection(addr.first, addr.second);
		if (!data) {
			if (requiredAcks != 0) main_.produceResponse(correlationId, KafkaConnector::BrokerNotAvailable, -1);
			fetchMetaData();
			return;
		}
		conn = new KafkaConnector::ProduceConnection(data, *this);
	}

	qint32 messageSetSize = 0;
	for (const Message & msg : messages) {
		messageSetSize +=
			8 + 4 +					// Offset, MessageSize
			4 + 1 + 1 +				// Crc, MagicByte, Attributes
			4 + msg.first.size() +	// Key
			4 + msg.second.size();	// Value
	}

	impl::KafkaRequestWriter req = conn->request(0, 0, correlationId,
		2 + 4 + 4 + 2 + topic.size() + 4 + 4 + 4 + messageSetSize);
	req
		<< (qint16)requiredAcks
		<< (qint32)ackTimeoutMs
		<< (qint32)1	// just one topic
		<< (impl::KafkaString)topic
		<< (qint32)1	// just one partition
		<< partitionId
		<< messageSetSize;
	for (const Message & msg : messages) {
		req
			<< (qint64)0	// Offset
			<< (qint32)(	// MessageSize
				4 + 1 + 1 +				// Crc, MagicByte, Attributes
				4 + msg.first.size() +	// Key
				4 + msg.second.size());	// Value
		const int crcPos = req.getCurrentSize();
		req
			<< (qint32)0	// Crc
			<< (qint8)0		// MagicByte
			<< (qint8)0		// Attributes
			<< msg.first	// Key
			<< msg.second;	// Value

		// calc CRC32
		qToBigEndian<qint32>(
			util::calcCRC32((const char *)req.getCurrentRawData() + crcPos + 4, req.getCurrentSize() - crcPos - 4),
			req.getCurrentRawData() + crcPos);
	}
	req.send();
}

void KafkaConnector::Impl::fetch(const QByteArray & topic, qint32 partitionId, qint64 offset,
	quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes, quint32 correlationId)
{
	if (!verifyThreadCall(&Impl::fetch, topic, partitionId, offset, maxWaitTime, minBytes, maxBytes, correlationId)) return;

	// get broker for topic and partition
	qint32 nodeId = responsibilities_[topic][partitionId].id;
	if (nodeId == -1) {
		main_.fetchResponse(correlationId, KafkaConnector::Messages(), -1, -1, KafkaConnector::UnknownTopicOrPartition);
		fetchMetaData();
		return;
	}

	// get tcp connection of broker
	KafkaConnector::FetchConnection *& conn = fetchConnections_[nodeId];
	if (!conn) {
		const KafkaConnector::Address & addr = allBrokers_[nodeId];
		TCPConnData * data = net_.openConnection(addr.first, addr.second);
		if (!data) {
			main_.fetchResponse(correlationId, KafkaConnector::Messages(), -1, -1, KafkaConnector::BrokerNotAvailable);
			fetchMetaData();
			return;
		}
		conn = new KafkaConnector::FetchConnection(data, *this);
	}

	impl::KafkaRequestWriter req = conn->request(1, 0, correlationId,
		4 + 4 + 4 + 4 + 2 + topic.size() + 4 + 4 + 8 + 4);
	req
		<< (qint32)-1
		<< (qint32)maxWaitTime
		<< (qint32)minBytes
		<< (qint32)1	// just one topic
		<< (impl::KafkaString)topic
		<< (qint32)1	// just one partition
		<< partitionId
		<< offset
		<< (qint32)maxBytes;
	req.send();
}

void KafkaConnector::Impl::joinGroup(const QByteArray & groupName, const QList<QByteArray> & topics)
{
//	Groups
//	ProtocolType = "consumer"
//	ProtocolName = "range"
//	ProtocolVersion = 0
//  UserData -> empty

}

void KafkaConnector::Impl::fetch(quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes)
{
}

void KafkaConnector::Impl::commit()
{
}

void KafkaConnector::Impl::leaveGroup()
{
}

}}	// namespace
