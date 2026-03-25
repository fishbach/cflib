/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::util { class ThreadVerify; }

namespace cflib::net {

class KafkaConnector
{
    CF_DISABLE_COPY(KafkaConnector)
public:
    enum State
    {
        Error      = -1,
        Idle       = 1,
        Connecting,
        Ready
    };

    enum ErrorCode
    {
        NoError                          = 0,
        Unknown                          = -1,
        OffsetOutOfRange                 = 1,
        InvalidMessage                   = 2,
        CorruptMessage                   = 2,
        UnknownTopicOrPartition          = 3,
        InvalidMessageSize               = 4,
        LeaderNotAvailable               = 5,
        NotLeaderForPartition            = 6,
        RequestTimedOut                  = 7,
        BrokerNotAvailable               = 8,
        ReplicaNotAvailable              = 9,
        MessageSizeTooLarge              = 10,
        StaleControllerEpochCode         = 11,
        OffsetMetadataTooLargeCode       = 12,
        GroupLoadInProgressCode             = 14,
        GroupCoordinatorNotAvailableCode = 15,
        NotCoordinatorForGroupCode       = 16,
        InvalidTopicCode                 = 17,
        RecordListTooLargeCode           = 18,
        NotEnoughReplicasCode            = 19,
        NotEnoughReplicasAfterAppendCode = 20,
        InvalidRequiredAcksCode          = 21,
        IllegalGenerationCode            = 22,
        InconsistentGroupProtocolCode    = 23,
        InvalidGroupIdCode               = 24,
        UnknownMemberIdCode              = 25,
        InvalidSessionTimeoutCode        = 26,
        RebalanceInProgressCode          = 27,
        InvalidCommitOffsetSizeCode      = 28,
        TopicAuthorizationFailedCode     = 29,
        GroupAuthorizationFailedCode     = 30,
        ClusterAuthorizationFailedCode   = 31
    };

    enum GroupAssignmentStrategy
    {
        UnknownAssignment    = -1,
        RangeAssignment      = 1,
        RoundRobinAssignment = 2
    };

    typedef Pair<ByteArray /* ip */, uint16 /* port */> Address;

    typedef Pair<ByteArray /* key */, ByteArray /* value */> Message;
    typedef List<Message> Messages;
    typedef List<ByteArray> Topics;

public:
    KafkaConnector(util::ThreadVerify * other = 0);
    virtual ~KafkaConnector();

    void connect(const ByteArray & destAddress, uint16 destPort);
    void connect(const List<Address> & cluster);

    // requiredAcks: 0 -> no response will be send / 1 -> wait for local write / -1 -> wait for all replicas
    // ackTimeoutMs: 0 -> wait for local write only / >0 -> max wait time for acks of replicas
    void produce(const ByteArray & topic, int32 partitionId, const Messages & messages,
        uint16 requiredAcks = 1, uint32 ackTimeoutMs = 0, uint32 correlationId = 1);

    // highwaterMarkOffset -> last offset + 1
    void getFirstOffset(const ByteArray & topic, int32 partitionId, uint32 correlationId = 1);
    void getHighwaterMarkOffset(const ByteArray & topic, int32 partitionId, uint32 correlationId = 1);

    void fetch(const ByteArray & topic, int32 partitionId, int64 offset,
        uint32 maxWaitTime = 0x7FFFFFFF, uint32 minBytes = 1, uint32 maxBytes = 0x100000 /* 1mb */, uint32 correlationId = 1);

    // only one group can be joined simultaneously
    void joinGroup(const ByteArray & groupId, const Topics & topics, GroupAssignmentStrategy preferredStrategy = RoundRobinAssignment);
    void fetch(uint32 maxWaitTime = 0x7FFFFFFF, uint32 minBytes = 1, uint32 maxBytes = 0x100000 /* 1mb */);
    void commit();    // commits last fetchResponse
    void leaveGroup();

protected:
    virtual void stateChanged(State state) { CF_UNUSED(state); }
    virtual void groupStateChanged(const Map<ByteArray, List<int32>> & responsibility) { CF_UNUSED(responsibility); }

    // offset -> is offset of first message appended to the kafka log
    virtual void produceResponse(uint32 correlationId, ErrorCode errorCode, int64 offset) {
        CF_UNUSED(correlationId); CF_UNUSED(errorCode); CF_UNUSED(offset); }

    virtual void offsetResponse(uint32 correlationId, int64 offset) {
        CF_UNUSED(correlationId); CF_UNUSED(offset); }

    // highwaterMarkOffset -> last offset + 1
    virtual void fetchResponse(uint32 correlationId, const Messages & messages,
        int64 firstOffset, int64 highwaterMarkOffset, ErrorCode errorCode) {
        CF_UNUSED(correlationId); CF_UNUSED(messages); CF_UNUSED(firstOffset); CF_UNUSED(highwaterMarkOffset); CF_UNUSED(errorCode); }

    virtual void fetchResponse(const Map<ByteArray, Messages> & messagesPerTopic, ErrorCode errorCode) {
        CF_UNUSED(messagesPerTopic); CF_UNUSED(errorCode); }

private:
    class MetadataConnection;
    class ProduceConnection;
    class OffsetConnection;
    class FetchConnection;
    class GroupConnection;
    class Impl;
    Impl * impl_;
};

} // namespace
