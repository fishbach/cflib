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
    void connect(const List<KafkaConnector::Address> & cluster);
    void fetchMetaData();

    void produce(const ByteArray & topic, int32 partitionId, const KafkaConnector::Messages & messages,
        uint16 requiredAcks, uint32 ackTimeoutMs, uint32 correlationId);
    void getOffsets(const ByteArray & topic, int32 partitionId, uint32 correlationId, bool first);
    void fetch(const ByteArray & topic, int32 partitionId, int64 offset,
        uint32 maxWaitTime, uint32 minBytes, uint32 maxBytes, uint32 correlationId);

    void joinGroup(const ByteArray & groupId, const KafkaConnector::Topics & topics, KafkaConnector::GroupAssignmentStrategy preferredStrategy);
    void fetch(uint32 maxWaitTime, uint32 minBytes, uint32 maxBytes);
    void commit();
    void leaveGroup();

    TCPConnData * connectToCluster();
    void rejoinGroup();
    void doJoin();
    void sendGroupHeartBeat();
    void doSync(const ByteArray & protocol, Map<ByteArray, Set<ByteArray>> memberTopics);
    Map<ByteArray, Map<ByteArray, List<int32>>> computeGroupAssignment(
        const ByteArray & protocol, Map<ByteArray, Set<ByteArray>> memberTopics);

public:
    KafkaConnector & main_;
    TCPManager net_;

    List<KafkaConnector::Address> cluster_;
    int clusterId_;

    Hash<int32 /* nodeId */, KafkaConnector::Address> allBrokers_;
    struct NodeId { int32 id; NodeId() : id(-1) {} };
    Map<ByteArray /* topic */, Map<int32 /* partitionId */, NodeId>> responsibilities_;

    KafkaConnector::State currentState_;

    Hash<int32 /* nodeId */, KafkaConnector::ProduceConnection *> produceConnections_;
    Hash<int32 /* nodeId */, KafkaConnector::FetchConnection   *> fetchConnections_;

    ByteArray groupId_;
    Map<ByteArray, List<int32>> groupTopicPartitions_;
    KafkaConnector::GroupAssignmentStrategy preferredStrategy_;

    KafkaConnector::MetadataConnection * groupCoordinatorRequest_;
    KafkaConnector::GroupConnection * groupConnection_;
    bool joinInProgress_;
    ByteArray groupMemberId_;
    int32 generationId_;
    util::EVTimer groupHeartbeatTimer_;
};

} // namespace
