/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
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

namespace cflib { namespace net {

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
    req << (qint32)0;    // no topic specified -> get all
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
            8 + 4 +                    // Offset, MessageSize
            4 + 1 + 1 +                // Crc, MagicByte, Attributes
            4 + msg.first.size() +    // Key
            4 + msg.second.size();    // Value
    }

    impl::KafkaRequestWriter req = conn->request(Produce, 0, correlationId,
        2 + 4 + 4 + 2 + topic.size() + 4 + 4 + 4 + messageSetSize);
    req
        << (qint16)requiredAcks
        << (qint32)ackTimeoutMs
        << (qint32)1    // just one topic
        << (impl::KafkaString)topic
        << (qint32)1    // just one partition
        << partitionId
        << messageSetSize;
    for (const Message & msg : messages) {
        req
            << (qint64)0    // Offset
            << (qint32)(    // MessageSize
                4 + 1 + 1 +                // Crc, MagicByte, Attributes
                4 + msg.first.size() +    // Key
                4 + msg.second.size());    // Value
        const int crcPos = req.getCurrentSize();
        req
            << (qint32)0    // Crc
            << (qint8)0        // MagicByte
            << (qint8)0        // Attributes
            << msg.first    // Key
            << msg.second;    // Value

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
        << (qint32)1    // just one topic
        << (impl::KafkaString)topic
        << (qint32)1    // just one partition
        << (qint32)partitionId
        << (qint64)(first ? -2 : -1);    // first or last offset
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
        << (qint32)1    // just one topic
        << (impl::KafkaString)topic
        << (qint32)1    // just one partition
        << partitionId
        << offset
        << (qint32)maxBytes;
    req.send();
}

void KafkaConnector::Impl::joinGroup(const QByteArray & groupId, const Topics & topics, GroupAssignmentStrategy preferredStrategy)
{
    if (!verifyThreadCall(&Impl::joinGroup, groupId, topics, preferredStrategy)) return;

    logFunctionTrace

    if (groupId_ != groupId) leaveGroup();
    groupId_ = groupId;
    groupTopicPartitions_.clear();
    for (const QByteArray & topic : topics) groupTopicPartitions_[topic] = QList<qint32>();
    preferredStrategy_ = preferredStrategy;
    rejoinGroup();
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

    qint32 metaDataSize = 2 + 4 + 4;
    for (const QByteArray & topic : groupTopicPartitions_.keys()) metaDataSize += 2 + topic.size();

    impl::KafkaRequestWriter req = groupConnection_->request(JoinGroup, 1, JoinGroup);
    req
        << (impl::KafkaString)groupId_
        << (qint32)6000                                // SessionTimeout   (see group.min.session.timeout.ms)
        << (qint32)6000                                // RebalanceTimeout (see group.min.session.timeout.ms)
        << (impl::KafkaString)groupMemberId_
        << (impl::KafkaString)"consumer"
        << (qint32)2                                // two protocols
        << (impl::KafkaString)(preferredStrategy_ == RangeAssignment ? "range" : "roundrobin")
        << metaDataSize
        << (qint16)0                                // Version
        << (qint32)groupTopicPartitions_.size();    // Topics
    for (const QByteArray & topic : groupTopicPartitions_.keys()) req << (impl::KafkaString)topic;
    req
        << QByteArray()                                // UserData
        << (impl::KafkaString)(preferredStrategy_ != RangeAssignment ? "range" : "roundrobin")
        << metaDataSize
        << (qint16)0                                // Version
        << (qint32)groupTopicPartitions_.size();    // Topics
    for (const QByteArray & topic : groupTopicPartitions_.keys()) req << (impl::KafkaString)topic;
    req
        << QByteArray();                            // UserData
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

void KafkaConnector::Impl::doSync(const QByteArray & protocol, QMap<QByteArray, QSet<QByteArray>> memberTopics)
{
    logFunctionTrace

    QMap<QByteArray, QMap<QByteArray, QList<qint32>>> groupAssignment = computeGroupAssignment(protocol, memberTopics);
    QMap<QByteArray, QByteArray> rawAssignment;
    for (const QByteArray & member : groupAssignment.keys()) {
        const QMap<QByteArray, QList<qint32>> & topics = groupAssignment[member];

        impl::KafkaRawWriter writer;
        writer
            << (qint16)0    // Version
            << (qint32)topics.size();

        for (const QByteArray & topic : topics.keys()) {
            const QList<qint32> & partitions = topics[topic];

            writer
                << (impl::KafkaString)topic
                << (qint32)partitions.size();
            for (qint32 partition : partitions) writer << partition;
        }
        writer
            << QByteArray();    // UserData

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

QMap<QByteArray, QMap<QByteArray, QList<qint32>>> KafkaConnector::Impl::computeGroupAssignment(
    const QByteArray & protocol, QMap<QByteArray, QSet<QByteArray>> memberTopics)
{
    QMap<QByteArray, QMap<QByteArray, QList<qint32>>> rv;
    if (memberTopics.isEmpty()) return rv;

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
            std::sort(members.begin(), members.end());

            int partitionCount = responsibilities_[topic].size();
            int partitionsPerMember  = partitionCount / members.size();
            int unassignedPartitions = partitionCount % members.size();
            int pId = 0;
            for (const QByteArray & member : members) {
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

        QSet<QByteArray> allTopics;
        for (const QSet<QByteArray> & topics : memberTopics.values()) allTopics += topics;

        QVector<QPair<QByteArray, qint32>> allTopicPartitions;
        for (const QByteArray & topic : allTopics) {
            for (qint32 i = 0 ; i < responsibilities_.value(topic).size() ; ++i) {
                allTopicPartitions << qMakePair(topic, i);
            }
        }
        std::sort(allTopicPartitions.begin(), allTopicPartitions.end());

        QList<QByteArray> members = memberTopics.keys();
        int memberId = 0;
        QMutableVectorIterator<QPair<QByteArray, qint32>> it(allTopicPartitions);
        while (it.hasNext()) {
            const QPair<QByteArray, qint32> & el = it.next();
            const QByteArray & member = members[memberId];
            if (memberTopics[member].contains(el.first)) {
                rv[member][el.first] << el.second;
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
    }

    return rv;
}

}}    // namespace
