/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/kafkaconnector.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class KafkaConnector::Impl : public util::ThreadVerify
{
public:
	enum ApiKeys {
		Produce            = 0,
		Fetch              = 1,
		Offsets            = 2,
		Metadata           = 3,
		LeaderAndIsr       = 4,
		StopReplica        = 5,
		UpdateMetadata     = 6,
		ControlledShutdown = 7,
		OffsetCommit       = 8,
		OffsetFetch        = 9,
		GroupCoordinator   = 10,
		JoinGroup          = 11,
		Heartbeat          = 12,
		LeaveGroup         = 13,
		SyncGroup          = 14,
		DescribeGroups     = 15,
		ListGroups         = 16,
		SaslHandshake      = 17,
		ApiVersions        = 18,
		CreateTopics       = 19,
		DeleteTopics       = 20
	};

public:
	Impl(KafkaConnector & main);
	Impl(KafkaConnector & parent, util::ThreadVerify * other);
	~Impl();

	void setState(KafkaConnector::State newState);
	void connect(const QList<KafkaConnector::Address> & cluster);
	void fetchMetaData();

	void produce(const QByteArray & topic, qint32 partitionId, const KafkaConnector::Messages & messages,
		quint16 requiredAcks, quint32 ackTimeoutMs, quint32 correlationId);
	void getOffsets(const QByteArray & topic, qint32 partitionId, quint32 correlationId, bool first);
	void fetch(const QByteArray & topic, qint32 partitionId, qint64 offset,
		quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes, quint32 correlationId);

	void joinGroup(const QByteArray & groupId, const KafkaConnector::Topics & topics, KafkaConnector::GroupAssignmentStrategy preferredStrategy);
	void fetch(quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes);
	void commit();
	void leaveGroup();

	TCPConnData * connectToCluster();
	void rejoinGroup();
	void doJoin();
	void sendGroupHeartBeat();
	void doSync(const QByteArray & protocol, QMap<QByteArray, QSet<QByteArray>> memberTopics);
	QMap<QByteArray, QMap<QByteArray, QList<qint32>>> computeGroupAssignment(
		const QByteArray & protocol, QMap<QByteArray, QSet<QByteArray>> memberTopics);

public:
	KafkaConnector & main_;
	TCPManager net_;

	QList<KafkaConnector::Address> cluster_;
	int clusterId_;

	QHash<qint32 /* nodeId */, KafkaConnector::Address> allBrokers_;
	struct NodeId { qint32 id; NodeId() : id(-1) {} };
	QMap<QByteArray /* topic */, QMap<qint32 /* partitionId */, NodeId>> responsibilities_;

	KafkaConnector::State currentState_;

	QHash<qint32 /* nodeId */, KafkaConnector::ProduceConnection *> produceConnections_;
	QHash<qint32 /* nodeId */, KafkaConnector::FetchConnection   *> fetchConnections_;

	QByteArray groupId_;
	QMap<QByteArray, QList<qint32>> groupTopicPartitions_;
	KafkaConnector::GroupAssignmentStrategy preferredStrategy_;

	KafkaConnector::MetadataConnection * groupCoordinatorRequest_;
	KafkaConnector::GroupConnection * groupConnection_;
	bool joinInProgress_;
	QByteArray groupMemberId_;
	qint32 generationId_;
	util::EVTimer groupHeartbeatTimer_;
};

}}	// namespace
