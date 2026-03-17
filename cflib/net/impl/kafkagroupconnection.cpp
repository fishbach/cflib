/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "kafkagroupconnection.h"

#include <cflib/util/log.h>
#include <cflib/util/timer.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

KafkaConnector::GroupConnection::GroupConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
    KafkaConnection(data),
    impl_(impl),
    leaving_(false)
{
}

void KafkaConnector::GroupConnection::reply(cfint32 correlationId, impl::KafkaRawReader & reader)
{
    if (correlationId == Impl::JoinGroup) {

        cfint16 errorCode;
        cfint32 generationId;
        impl::KafkaString groupProtocol;
        impl::KafkaString leaderId;
        impl::KafkaString ownMemberId;
        reader >> errorCode >> generationId >> groupProtocol >> leaderId >> ownMemberId;

        CFMap<CFByteArray, CFSet<CFByteArray>> memberTopics;

        cfint32 memberCount;
        reader >> memberCount;
        for (cfint32 i = 0 ; i < memberCount ; ++i) {
            impl::KafkaString memberId;
            cfint32 metaDataSize;
            cfint16 version;
            reader >> memberId >> metaDataSize >> version;

            cfint32 topicCount;
            reader >> topicCount;
            for (cfint32 i = 0 ; i < topicCount ; ++i) {
                impl::KafkaString topic;
                reader >> topic;

                memberTopics[CFByteArray(memberId)].insert(CFByteArray(topic));
            }

            CFByteArray userData;
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

        cfint16 errorCode;
        reader >> errorCode;
        if (errorCode != KafkaConnector::NoError) {
            logInfo("got heartbeat error: %1", errorCode);
            impl_.rejoinGroup();
        }

    } else if (correlationId == Impl::SyncGroup) {

        for (auto & [key, val] : impl_.groupTopicPartitions_) val.clear();

        cfint16 errorCode;
        cfint32 memberAssignmentSize;
        cfint16 version;
        reader >> errorCode >> memberAssignmentSize >> version;

        cfint32 topicCount;
        reader >> topicCount;
        for (cfint32 i = 0 ; i < topicCount ; ++i) {
            impl::KafkaString topic;
            reader >> topic;

            CFList<cfint32> & partitions = impl_.groupTopicPartitions_[topic];
            cfint32 partitionCount;
            reader >> partitionCount;
            for (cfint32 i = 0 ; i < partitionCount ; ++i) {
                cfint32 partition;
                reader >> partition;
                if (errorCode == KafkaConnector::NoError) partitions << partition;
            }
        }

        CFByteArray userData;
        reader >> userData;

        impl_.main_.groupStateChanged(impl_.groupTopicPartitions_);
        impl_.joinInProgress_ = false;
        if (errorCode != KafkaConnector::NoError) close();

    } else if (correlationId == Impl::LeaveGroup) {

        cfint16 errorCode;
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

}}    // namespace
