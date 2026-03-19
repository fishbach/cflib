/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
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

namespace cflib::net {

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
    void connect(const CFList<KafkaConnector::Address> & cluster);
    void fetchMetaData();

    void produce(const ByteArray & topic, cfint32 partitionId, const KafkaConnector::Messages & messages,
        cfuint16 requiredAcks, cfuint32 ackTimeoutMs, cfuint32 correlationId);
    void getOffsets(const ByteArray & topic, cfint32 partitionId, cfuint32 correlationId, bool first);
    void fetch(const ByteArray & topic, cfint32 partitionId, cfint64 offset,
        cfuint32 maxWaitTime, cfuint32 minBytes, cfuint32 maxBytes, cfuint32 correlationId);

    void joinGroup(const ByteArray & groupId, const KafkaConnector::Topics & topics, KafkaConnector::GroupAssignmentStrategy preferredStrategy);
    void fetch(cfuint32 maxWaitTime, cfuint32 minBytes, cfuint32 maxBytes);
    void commit();
    void leaveGroup();

    TCPConnData * connectToCluster();
    void rejoinGroup();
    void doJoin();
    void sendGroupHeartBeat();
    void doSync(const ByteArray & protocol, CFMap<ByteArray, CFSet<ByteArray>> memberTopics);
    CFMap<ByteArray, CFMap<ByteArray, CFList<cfint32>>> computeGroupAssignment(
        const ByteArray & protocol, CFMap<ByteArray, CFSet<ByteArray>> memberTopics);

public:
    KafkaConnector & main_;
    TCPManager net_;

    CFList<KafkaConnector::Address> cluster_;
    int clusterId_;

    CFHash<cfint32 /* nodeId */, KafkaConnector::Address> allBrokers_;
    struct NodeId { cfint32 id; NodeId() : id(-1) {} };
    CFMap<ByteArray /* topic */, CFMap<cfint32 /* partitionId */, NodeId>> responsibilities_;

    KafkaConnector::State currentState_;

    CFHash<cfint32 /* nodeId */, KafkaConnector::ProduceConnection *> produceConnections_;
    CFHash<cfint32 /* nodeId */, KafkaConnector::FetchConnection   *> fetchConnections_;

    ByteArray groupId_;
    CFMap<ByteArray, CFList<cfint32>> groupTopicPartitions_;
    KafkaConnector::GroupAssignmentStrategy preferredStrategy_;

    KafkaConnector::MetadataConnection * groupCoordinatorRequest_;
    KafkaConnector::GroupConnection * groupConnection_;
    bool joinInProgress_;
    ByteArray groupMemberId_;
    cfint32 generationId_;
    util::EVTimer groupHeartbeatTimer_;
};

} // namespace
