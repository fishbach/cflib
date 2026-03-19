/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "kafkagroupconnection.h"

#include <cflib/util/log.h>
#include <cflib/util/timer.h>

USE_LOG(LogCat::Network)

namespace cflib::net {

KafkaConnector::GroupConnection::GroupConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
    KafkaConnection(data),
    impl_(impl),
    leaving_(false)
{
}

void KafkaConnector::GroupConnection::reply(int32 correlationId, impl::KafkaRawReader & reader)
{
    if (correlationId == Impl::JoinGroup) {

        int16 errorCode;
        int32 generationId;
        impl::KafkaString groupProtocol;
        impl::KafkaString leaderId;
        impl::KafkaString ownMemberId;
        reader >> errorCode >> generationId >> groupProtocol >> leaderId >> ownMemberId;

        Map<ByteArray, Set<ByteArray>> memberTopics;

        int32 memberCount;
        reader >> memberCount;
        for (int32 i = 0 ; i < memberCount ; ++i) {
            impl::KafkaString memberId;
            int32 metaDataSize;
            int16 version;
            reader >> memberId >> metaDataSize >> version;

            int32 topicCount;
            reader >> topicCount;
            for (int32 i = 0 ; i < topicCount ; ++i) {
                impl::KafkaString topic;
                reader >> topic;

                memberTopics[ByteArray(memberId)].insert(ByteArray(topic));
            }

            ByteArray userData;
            reader >> userData;
        }

        if (errorCode != KafkaConnector::NoError) {
            logWarn("cannot join group: %1", errorCode);
            impl_.generationId_ = 0;
            close();
        } else {
            impl_.generationId_ = generationId;
            impl_.groupMemberId_ = ownMemberId;
            impl_.doSync(groupProtocol, memberTopics);
            impl_.groupHeartbeatTimer_.start(1.0);
        }

    } else if (correlationId == Impl::Heartbeat) {

        int16 errorCode;
        reader >> errorCode;
        if (errorCode != KafkaConnector::NoError) {
            logInfo("got heartbeat error: %1", errorCode);
            impl_.rejoinGroup();
        }

    } else if (correlationId == Impl::SyncGroup) {

        for (auto & [key, val] : impl_.groupTopicPartitions_) val.clear();

        int16 errorCode;
        int32 memberAssignmentSize;
        int16 version;
        reader >> errorCode >> memberAssignmentSize >> version;

        int32 topicCount;
        reader >> topicCount;
        for (int32 i = 0 ; i < topicCount ; ++i) {
            impl::KafkaString topic;
            reader >> topic;

            List<int32> & partitions = impl_.groupTopicPartitions_[topic];
            int32 partitionCount;
            reader >> partitionCount;
            for (int32 i = 0 ; i < partitionCount ; ++i) {
                int32 partition;
                reader >> partition;
                if (errorCode == KafkaConnector::NoError) partitions << partition;
            }
        }

        ByteArray userData;
        reader >> userData;

        impl_.main_.groupStateChanged(impl_.groupTopicPartitions_);
        impl_.joinInProgress_ = false;
        if (errorCode != KafkaConnector::NoError) close();

    } else if (correlationId == Impl::LeaveGroup) {

        int16 errorCode;
        reader >> errorCode;
        if (errorCode != KafkaConnector::NoError) {
            logWarn("group leave error: %1", errorCode);
        }

        leaving_ = true;
        close();

    }
}

void KafkaConnector::GroupConnection::closed()
{
    if (leaving_) return;

    impl_.groupHeartbeatTimer_.stop();
    impl_.groupConnection_ = 0;
    if (!impl_.groupId_.isEmpty()) {
        util::Timer::singleShot(1.0, &impl_, &KafkaConnector::Impl::rejoinGroup);
    }
}

} // namespace
