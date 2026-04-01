/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

namespace cflib::net {

KafkaConnector::Impl::Impl(KafkaConnector &main) :
    ThreadVerify("KafkaConnector", ThreadVerify::Net),
    main_(main),
    net_(0, this),
    clusterId_(0),
    currentState_(KafkaConnector::Idle),
    preferredStrategy_(UnknownAssignment),
    groupCoordinatorRequest_(0),
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
    preferredStrategy_(UnknownAssignment),
    groupCoordinatorRequest_(0),
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

void KafkaConnector::Impl::connect(const List<KafkaConnector::Address> &cluster)
{
    if (!verifyThreadCall(&Impl::connect, cluster)) return;

    if (!cluster_.empty()) {
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
        if (cluster_.empty()) {
            setState(Idle);
        } else {
            logWarn("could not connect to kafka cluster");
            util::Timer::singleShot(1.0, this, &Impl::fetchMetaData);
        }
        return;
    }

    KafkaConnector::MetadataConnection * conn = new KafkaConnector::MetadataConnection(true, data, *this);
    impl::KafkaRequestWriter req = conn->request(Metadata, 0, 1, 4);
    req << (int32)0;    // no topic specified -> get all
    req.send();
}

void KafkaConnector::Impl::produce(const ByteArray & topic, int32 partitionId, const Messages & messages,
    uint16 requiredAcks, uint32 ackTimeoutMs, uint32 correlationId)
{
    if (!verifyThreadCall(&Impl::produce, topic, partitionId, messages, requiredAcks, ackTimeoutMs, correlationId)) return;

    // get broker for topic and partition
    int32 nodeId = responsibilities_[topic][partitionId].id;
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

    int32 messageSetSize = 0;
    for (const Message & msg : messages) {
        messageSetSize +=
            8 + 4 +                    // Offset, MessageSize
            4 + 1 + 1 +                // Crc, MagicByte, Attributes
            4 + msg.first.size() +    // Key
            4 + msg.second.size();    // Value
    }

    impl::KafkaRequestWriter req = conn->request(Produce, 0, correlationId,
        2 + 4 + 4 + 2 + topic.size() + 4 + 4 + 4 + messageSetSize);
    req
        << (int16)requiredAcks
        << (int32)ackTimeoutMs
        << (int32)1    // just one topic
        << (impl::KafkaString)topic
        << (int32)1    // just one partition
        << partitionId
        << messageSetSize;
    for (const Message & msg : messages) {
        req
            << (int64)0    // Offset
            << (int32)(    // MessageSize
                4 + 1 + 1 +                // Crc, MagicByte, Attributes
                4 + msg.first.size() +    // Key
                4 + msg.second.size());    // Value
        const int crcPos = req.getCurrentSize();
        req
            << (int32)0    // Crc
            << (int8)0        // MagicByte
            << (int8)0        // Attributes
            << msg.first    // Key
            << msg.second;    // Value

        // calc CRC32
        cfToBigEndian<int32>(
            util::calcCRC32((const char *)req.getCurrentRawData() + crcPos + 4, req.getCurrentSize() - crcPos - 4),
            req.getCurrentRawData() + crcPos);
    }
    req.send();
}

void KafkaConnector::Impl::getOffsets(const ByteArray & topic, int32 partitionId, uint32 correlationId, bool first)
{
    if (!verifyThreadCall(&Impl::getOffsets, topic, partitionId, correlationId, first)) return;

    logFunctionTrace

    // get broker for topic and partition
    int32 nodeId = responsibilities_[topic][partitionId].id;
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
        << (int32)-1
        << (int32)1    // just one topic
        << (impl::KafkaString)topic
        << (int32)1    // just one partition
        << (int32)partitionId
        << (int64)(first ? -2 : -1);    // first or last offset
    req.send();
}

void KafkaConnector::Impl::fetch(const ByteArray & topic, int32 partitionId, int64 offset,
    uint32 maxWaitTime, uint32 minBytes, uint32 maxBytes, uint32 correlationId)
{
    if (!verifyThreadCall(&Impl::fetch, topic, partitionId, offset, maxWaitTime, minBytes, maxBytes, correlationId)) return;

    logFunctionTrace

    // get broker for topic and partition
    int32 nodeId = responsibilities_[topic][partitionId].id;
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
        << (int32)-1
        << (int32)maxWaitTime
        << (int32)minBytes
        << (int32)1    // just one topic
        << (impl::KafkaString)topic
        << (int32)1    // just one partition
        << partitionId
        << offset
        << (int32)maxBytes;
    req.send();
}

void KafkaConnector::Impl::joinGroup(const ByteArray & groupId, const Topics & topics, GroupAssignmentStrategy preferredStrategy)
{
    if (!verifyThreadCall(&Impl::joinGroup, groupId, topics, preferredStrategy)) return;

    logFunctionTrace

    if (groupId_ != groupId) leaveGroup();
    groupId_ = groupId;
    groupTopicPartitions_.clear();
    for (const ByteArray & topic : topics) groupTopicPartitions_[topic] = List<int32>();
    preferredStrategy_ = preferredStrategy;
    rejoinGroup();
}

void KafkaConnector::Impl::fetch(uint32 maxWaitTime, uint32 minBytes, uint32 maxBytes)
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

    // send clean leave
    if (groupConnection_ && !groupId_.isEmpty() && !groupMemberId_.isEmpty()) {
        impl::KafkaRequestWriter req = groupConnection_->request(LeaveGroup, 0, LeaveGroup);
        req
            << (impl::KafkaString)groupId_
            << (impl::KafkaString)groupMemberId_;
        req.send();
        groupHeartbeatTimer_.stop();
        groupConnection_ = 0;
    }

    groupId_.clear();
    if (groupCoordinatorRequest_) groupCoordinatorRequest_->close();
    if (groupConnection_) groupConnection_->close();
    joinInProgress_ = false;
}

TCPConnData * KafkaConnector::Impl::connectToCluster()
{
    logFunctionTrace

    if (cluster_.empty()) return 0;
    if (clusterId_ >= (ssize_t)cluster_.size()) clusterId_ = 0;

    const int startId = clusterId_;
    do {
        const KafkaConnector::Address & addr = cluster_[clusterId_];
        TCPConnData * data = net_.openConnection(addr.first, addr.second);
        if (data) return data;

        if (++clusterId_ >= (ssize_t)cluster_.size()) clusterId_ = 0;
    } while (clusterId_ != startId);

    return 0;
}

void KafkaConnector::Impl::rejoinGroup()
{
    logFunctionTrace

    if (joinInProgress_ || groupId_.isEmpty()) return;
    joinInProgress_ = true;

    if (!groupConnection_) {
        TCPConnData * data = connectToCluster();
        if (!data) {
            groupId_.clear();
            logWarn("could not get group coordinator");
            return;
        }

        groupCoordinatorRequest_ = new KafkaConnector::MetadataConnection(false, data, *this);
        impl::KafkaRequestWriter req = groupCoordinatorRequest_->request(GroupCoordinator);
        req << (impl::KafkaString)groupId_;
        req.send();
    } else {
        doJoin();
    }
}

void KafkaConnector::Impl::doJoin()
{
    logFunctionTrace

    int32 metaDataSize = 2 + 4 + 4;
    for (const ByteArray & topic : groupTopicPartitions_.keys()) metaDataSize += 2 + topic.size();

    impl::KafkaRequestWriter req = groupConnection_->request(JoinGroup, 1, JoinGroup);
    req
        << (impl::KafkaString)groupId_
        << (int32)6000                                // SessionTimeout   (see group.min.session.timeout.ms)
        << (int32)6000                                // RebalanceTimeout (see group.min.session.timeout.ms)
        << (impl::KafkaString)groupMemberId_
        << (impl::KafkaString)"consumer"
        << (int32)2                                // two protocols
        << (impl::KafkaString)(preferredStrategy_ == RangeAssignment ? "range" : "roundrobin")
        << metaDataSize
        << (int16)0                                // Version
        << (int32)groupTopicPartitions_.size();    // Topics
    for (const ByteArray & topic : groupTopicPartitions_.keys()) req << (impl::KafkaString)topic;
    req
        << ByteArray()                                // UserData
        << (impl::KafkaString)(preferredStrategy_ != RangeAssignment ? "range" : "roundrobin")
        << metaDataSize
        << (int16)0                                // Version
        << (int32)groupTopicPartitions_.size();    // Topics
    for (const ByteArray & topic : groupTopicPartitions_.keys()) req << (impl::KafkaString)topic;
    req
        << ByteArray();                            // UserData
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

void KafkaConnector::Impl::doSync(const ByteArray & protocol, Map<ByteArray, Set<ByteArray>> memberTopics)
{
    logFunctionTrace

    Map<ByteArray, Map<ByteArray, List<int32>>> groupAssignment = computeGroupAssignment(protocol, memberTopics);
    Map<ByteArray, ByteArray> rawAssignment;
    for (const ByteArray & member : groupAssignment.keys()) {
        const Map<ByteArray, List<int32>> & topics = groupAssignment[member];

        impl::KafkaRawWriter writer;
        writer
            << (int16)0    // Version
            << (int32)topics.size();

        for (const auto & [topic, partitions] : topics) {

            writer
                << (impl::KafkaString)topic
                << (int32)partitions.size();
            for (int32 partition : partitions) writer << partition;
        }
        writer
            << ByteArray();    // UserData

        rawAssignment[member] = writer.getRawContent();
    }

    impl::KafkaRequestWriter req = groupConnection_->request(SyncGroup, 0, SyncGroup);
    req
        << (impl::KafkaString)groupId_
        << generationId_
        << (impl::KafkaString)groupMemberId_
        << (int32)rawAssignment.size();
    for (const ByteArray & member : rawAssignment.keys()) {
        const ByteArray & assignment = rawAssignment[member];
        req
            << (impl::KafkaString)member
            << assignment;
    }
    req.send();
}

Map<ByteArray, Map<ByteArray, List<int32>>> KafkaConnector::Impl::computeGroupAssignment(
    const ByteArray & protocol, Map<ByteArray, Set<ByteArray>> memberTopics)
{
    Map<ByteArray, Map<ByteArray, List<int32>>> rv;
    if (memberTopics.empty()) return rv;

    logFunctionTraceParam("computeGroupAssignment with %1", protocol);

    if (protocol == "range") {

        Map<ByteArray, List<ByteArray>> topicMembers;
        for (const ByteArray & member : memberTopics.keys()) {
            for (const ByteArray & topic : memberTopics[member]) {
                if (!responsibilities_.value(topic).empty()) topicMembers[topic] << member;
            }
        }

        for (const ByteArray & topic : topicMembers.keys()) {
            List<ByteArray> & members = topicMembers[topic];
            std::sort(members.begin(), members.end());

            int partitionCount = responsibilities_[topic].size();
            int partitionsPerMember  = partitionCount / members.size();
            int unassignedPartitions = partitionCount % members.size();
            int pId = 0;
            for (const ByteArray & member : members) {
                for (int j = 0 ; j < partitionsPerMember ; ++j) {
                    rv[member][topic] << pId++;
                }
                if (unassignedPartitions > 0) {
                    --unassignedPartitions;
                    rv[member][topic] << pId++;
                }
            }
        }

    } else if (protocol == "roundrobin") {

        Set<ByteArray> allTopics;
        for (const auto & [member, topics] : memberTopics) allTopics += topics;

        List<Pair<ByteArray, int32>> allTopicPartitions;
        for (const ByteArray & topic : allTopics) {
            for (int32 i = 0 ; i < (int32)responsibilities_.value(topic).size() ; ++i) {
                allTopicPartitions << Pair(topic, i);
            }
        }
        std::sort(allTopicPartitions.begin(), allTopicPartitions.end());

        List<ByteArray> members = memberTopics.keys();
        int memberId = 0;
        while (!allTopicPartitions.empty()) {
            bool assigned = false;
            for (auto it = allTopicPartitions.begin(); it != allTopicPartitions.end(); ++it) {
                const ByteArray & member = members[memberId];
                if (memberTopics[member].contains(it->first)) {
                    rv[member][it->first] << it->second;
                    logInfo("assigning %1/%2 to %3", it->first, it->second, member);
                    allTopicPartitions.erase(it);
                    assigned = true;
                    break;
                }
            }
            if (!assigned) break;
            if (++memberId >= (int)members.size()) memberId = 0;
        }

    } else {
        logWarn("unknown protocol: %1", protocol);
    }

    return rv;
}

} // namespace
