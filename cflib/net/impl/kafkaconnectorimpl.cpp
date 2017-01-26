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

#include "kafkaconnectorimpl.h"

#include <cflib/net/impl/kafkafetchconnection.h>
#include <cflib/net/impl/kafkagroupconnection.h>
#include <cflib/net/impl/kafkametadataconnection.h>
#include <cflib/net/impl/kafkaoffsetconnection.h>
#include <cflib/net/impl/kafkaproduceconnection.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

KafkaConnector::Impl::Impl(KafkaConnector &main) :
	ThreadVerify("KafkaConnector", ThreadVerify::Net),
	main_(main),
	net_(0, this),
	clusterId_(0),
	currentState_(KafkaConnector::Idle),
	groupConnection_(0),
	joinInProgress_(false),
	groupMemberId_(""),
	generationId_(0),
	groupHeartbeatTimer_(this, &Impl::sendGroupHeartBeat)
{
}

KafkaConnector::Impl::Impl(KafkaConnector &parent, util::ThreadVerify *other) :
	ThreadVerify(other),
	main_(parent),
	net_(0, this),
	clusterId_(0),
	currentState_(KafkaConnector::Idle),
	groupConnection_(0),
	joinInProgress_(false),
	groupMemberId_(""),
	generationId_(0),
	groupHeartbeatTimer_(this, &Impl::sendGroupHeartBeat)
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

	TCPConnData * data = connectToCluster();
	if (!data) {
		if (cluster_.isEmpty()) {
			setState(Idle);
		} else {
			logWarn("could not connect to kafka cluster");
			util::Timer::singleShot(1.0, this, &Impl::fetchMetaData);
		}
		return;
	}

	KafkaConnector::MetadataConnection * conn = new KafkaConnector::MetadataConnection(true, data, *this);
	impl::KafkaRequestWriter req = conn->request(Metadata, 0, 1, 4);
	req << (qint32)0;	// no topic specified -> get all
	req.send();
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

	impl::KafkaRequestWriter req = conn->request(Produce, 0, correlationId,
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

void KafkaConnector::Impl::getOffsets(const QByteArray & topic, qint32 partitionId, quint32 correlationId, bool first)
{
	if (!verifyThreadCall(&Impl::getOffsets, topic, partitionId, correlationId, first)) return;

	logFunctionTrace

	// get broker for topic and partition
	qint32 nodeId = responsibilities_[topic][partitionId].id;
	if (nodeId == -1) {
		main_.offsetResponse(correlationId, -1);
		fetchMetaData();
		return;
	}

	// get tcp connection of broker
	const KafkaConnector::Address & addr = allBrokers_[nodeId];
	TCPConnData * data = net_.openConnection(addr.first, addr.second);
	if (!data) {
		main_.offsetResponse(correlationId, -1);
		fetchMetaData();
		return;
	}

	KafkaConnector::OffsetConnection * conn = new KafkaConnector::OffsetConnection(data, *this);
	impl::KafkaRequestWriter req = conn->request(Offsets, 1, correlationId,
		4 + 2 + topic.size() + 4 + 8 + 4);
	req
		<< (qint32)-1
		<< (qint32)1	// just one topic
		<< (impl::KafkaString)topic
		<< (qint32)1	// just one partition
		<< (qint32)partitionId
		<< (qint64)(first ? -2 : -1);	// first or last offset
	req.send();
}

void KafkaConnector::Impl::fetch(const QByteArray & topic, qint32 partitionId, qint64 offset,
	quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes, quint32 correlationId)
{
	if (!verifyThreadCall(&Impl::fetch, topic, partitionId, offset, maxWaitTime, minBytes, maxBytes, correlationId)) return;

	logFunctionTrace

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

	impl::KafkaRequestWriter req = conn->request(Fetch, 0, correlationId,
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

void KafkaConnector::Impl::joinGroup(const QByteArray & groupId, const Topics & topics, GroupAssignmentStrategy preferredStrategy)
{
	if (!verifyThreadCall(&Impl::joinGroup, groupId, topics, preferredStrategy)) return;

	logFunctionTrace

	if (joinInProgress_) {
		logWarn("cannot join twice");
		return;
	}

	if (groupId != groupId_) leaveGroup();
	groupId_ = groupId;
	groupTopicPartitions_.clear();
	for (const QByteArray & topic : topics) groupTopicPartitions_[topic].clear();
	preferredStrategy_ = preferredStrategy;
	doJoin();
}

void KafkaConnector::Impl::rejoinGroup()
{
	if (joinInProgress_) return;
	joinInProgress_ = true;

	QMutableMapIterator<QByteArray, QList<qint32>> it(groupTopicPartitions_);
	while (it.hasNext()) it.next().value().clear();
	doJoin();
}

void KafkaConnector::Impl::fetch(quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes)
{
	if (!verifyThreadCall(&Impl::fetch, maxWaitTime, minBytes, maxBytes)) return;

	// todo: ensure that join / sync has finished

}

void KafkaConnector::Impl::commit()
{
	if (!verifyThreadCall(&Impl::commit)) return;

}

void KafkaConnector::Impl::leaveGroup()
{
	if (!verifyThreadCall(&Impl::leaveGroup)) return;
}

TCPConnData * KafkaConnector::Impl::connectToCluster()
{
	logFunctionTrace

	if (cluster_.isEmpty()) return 0;
	if (clusterId_ >= cluster_.size()) clusterId_ = 0;

	const int startId = clusterId_;
	do {
		const KafkaConnector::Address & addr = cluster_[clusterId_];
		TCPConnData * data = net_.openConnection(addr.first, addr.second);
		if (data) return data;

		if (++clusterId_ >= cluster_.size()) clusterId_ = 0;
	} while (clusterId_ != startId);

	return 0;
}

void KafkaConnector::Impl::doJoin()
{
	logFunctionTrace

	// ensure group connection is available
	if (!groupConnection_) {
		TCPConnData * data = connectToCluster();
		if (!data) {
			groupId_.clear();
			logWarn("could not get group coordinator");
			return;
		}

		KafkaConnector::MetadataConnection * conn = new KafkaConnector::MetadataConnection(false, data, *this);
		impl::KafkaRequestWriter req = conn->request(GroupCoordinator, 0, 1, 2 + groupId_.size());
		req << (impl::KafkaString)groupId_;
		req.send();
		return;
	}

	qint32 metaDataSize = 2 + 4 + 4;
	for (const QByteArray & topic : groupTopicPartitions_.keys()) metaDataSize += 2 + topic.size();

	impl::KafkaRequestWriter req = groupConnection_->request(JoinGroup, 1, JoinGroup);
	req
		<< (impl::KafkaString)groupId_
		<< (qint32)10000							// SessionTimeout (ms)
		<< (qint32)10000							// RebalanceTimeout (ms)
		<< (impl::KafkaString)groupMemberId_
		<< (impl::KafkaString)"consumer"
		<< (qint32)2								// two protocols
		<< (impl::KafkaString)(preferredStrategy_ == RangeAssignment ? "range" : "roundrobin")
		<< metaDataSize
		<< (qint16)0								// Version
		<< (qint32)groupTopicPartitions_.size();	// Topics
	for (const QByteArray & topic : groupTopicPartitions_.keys()) req << (impl::KafkaString)topic;
	req
		<< QByteArray()								// UserData
		<< (impl::KafkaString)(preferredStrategy_ != RangeAssignment ? "range" : "roundrobin")
		<< metaDataSize
		<< (qint16)0								// Version
		<< (qint32)groupTopicPartitions_.size();	// Topics
	for (const QByteArray & topic : groupTopicPartitions_.keys()) req << (impl::KafkaString)topic;
	req
		<< QByteArray();							// UserData
	req.send();
}

void KafkaConnector::Impl::sendGroupHeartBeat()
{
	logFunctionTrace

	impl::KafkaRequestWriter req = groupConnection_->request(Heartbeat, 0, Heartbeat);
	req
		<< (impl::KafkaString)groupId_
		<< generationId_
		<< (impl::KafkaString)groupMemberId_;
	req.send();
}

void KafkaConnector::Impl::doSync()
{
	logFunctionTrace

	QMap<QByteArray, QByteArray> rawAssignment;
	for (const QByteArray & member : groupAssignment_.keys()) {
		const QMap<QByteArray, QList<qint32>> & topics = groupAssignment_[member];

		impl::KafkaRawWriter writer;
		writer
			<< (qint16)0	// Version
			<< (qint32)topics.size();

		for (const QByteArray & topic : topics.keys()) {
			const QList<qint32> & partitions = topics[topic];

			writer
				<< (impl::KafkaString)topic
				<< (qint32)partitions.size();
			for (qint32 partition : partitions) writer << partition;
		}
		writer
			<< QByteArray();	// UserData

		rawAssignment[member] = writer.getRawContent();
	}

	impl::KafkaRequestWriter req = groupConnection_->request(SyncGroup, 0, SyncGroup);
	req
		<< (impl::KafkaString)groupId_
		<< generationId_
		<< (impl::KafkaString)groupMemberId_
		<< (qint32)rawAssignment.size();
	for (const QByteArray & member : rawAssignment.keys()) {
		const QByteArray & assignment = rawAssignment[member];
		req
			<< (impl::KafkaString)member
			<< assignment;
	}
	req.send();
}

void KafkaConnector::Impl::computeGroupAssignment(const QByteArray & protocol, QMap<QByteArray, QSet<QByteArray>> memberTopics)
{
	groupAssignment_.clear();
	if (memberTopics.isEmpty()) return;

	logFunctionTraceParam("computeGroupAssignment with %1", protocol);

	if (protocol == "range") {

		QMap<QByteArray, QList<QByteArray>> topicMembers;
		for (const QByteArray & member : memberTopics.keys()) {
			for (const QByteArray & topic : memberTopics[member]) {
				if (!responsibilities_.value(topic).isEmpty()) topicMembers[topic] << member;
			}
		}

		for (const QByteArray & topic : topicMembers.keys()) {
			QList<QByteArray> & members = topicMembers[topic];
			qSort(members);

			int partitionCount = responsibilities_[topic].size();
			int partitionsPerMember  = partitionCount / members.size();
			int unassignedPartitions = partitionCount % members.size();
			int pId = 0;
			for (const QByteArray & member : members) {
				for (int j = 0 ; j < partitionsPerMember ; ++j) {
					groupAssignment_[member][topic] << pId++;
				}
				if (unassignedPartitions > 0) {
					--unassignedPartitions;
					groupAssignment_[member][topic] << pId++;
				}
			}
		}

	} else if (protocol == "roundrobin") {

		QSet<QByteArray> allTopics;
		for (const QSet<QByteArray> & topics : memberTopics.values()) allTopics += topics;

		QVector<QPair<QByteArray, qint32>> allTopicPartitions;
		for (const QByteArray & topic : allTopics) {
			for (qint32 i = 0 ; i < responsibilities_.value(topic).size() ; ++i) {
				allTopicPartitions << qMakePair(topic, i);
			}
		}
		qSort(allTopicPartitions);

		QList<QByteArray> members = memberTopics.keys();
		int memberId = 0;
		QMutableVectorIterator<QPair<QByteArray, qint32>> it(allTopicPartitions);
		while (it.hasNext()) {
			const QPair<QByteArray, qint32> & el = it.next();
			const QByteArray & member = members[memberId];
			if (memberTopics[member].contains(el.first)) {
				groupAssignment_[member][el.first] << el.second;
				logInfo("assigning %1/%2 to %3", el.first, el.second, member);
				it.remove();
			} else if (it.hasNext()) {
				// try next (topic, partition) for member
				continue;
			}

			if (++memberId >= members.size()) memberId = 0;
			it.toFront();
		}

	} else {
		logWarn("unknown protocol: %1", protocol);
		generationId_ = 0;
		groupConnection_->abort();
	}
}

}}	// namespace
