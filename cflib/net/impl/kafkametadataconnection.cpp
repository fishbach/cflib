/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "kafkametadataconnection.h"

#include <cflib/net/impl/kafkagroupconnection.h>
#include <cflib/util/log.h>
#include <cflib/util/timer.h>

USE_LOG(LogCat::Network)

namespace cflib::net {

KafkaConnector::MetadataConnection::MetadataConnection(bool isMetaDataRequest, TCPConnData * data, KafkaConnector::Impl & impl) :
    KafkaConnection(data),
    impl_(impl),
    isMetaDataRequest_(isMetaDataRequest)
{
}

void KafkaConnector::MetadataConnection::reply(int32, impl::KafkaRawReader & reader)
{
    if (isMetaDataRequest_) readMetaData        (reader);
    else                    readGroupCoordinator(reader);
}

void KafkaConnector::MetadataConnection::closed()
{
    if (isMetaDataRequest_) {
        for (const auto & [nodeId, addr_unused] : impl_.allBrokers_) {
            const KafkaConnector::Address & addr = impl_.allBrokers_[nodeId];
            logInfo("found broker %1 with ip %2 (port: %3)", nodeId, addr.first, addr.second);
        }

        for (const auto & [topic, partMap] : impl_.responsibilities_) {
            ByteArray partitionStr;
            bool isFirst = true;
            for (const auto & [partitionId, nodeInfo] : partMap) {
                if (isFirst) isFirst = false;
                else         partitionStr += ' ';
                partitionStr += ByteArray::fromInt(partitionId);
            }
            logInfo("found topic \"%1\" (partitions: %2)", topic, partitionStr);
        }

        if (impl_.allBrokers_.isEmpty()) {
            logWarn("could not retrieve kafka cluster meta data");
            ++impl_.clusterId_;
            util::Timer::singleShot(1.0, &impl_, &Impl::fetchMetaData);
        } else {
            impl_.setState(Ready);
        }
    } else {
        impl_.groupCoordinatorRequest_ = 0;
        if (!impl_.groupId_.isEmpty() && !impl_.groupConnection_) {
            logWarn("could not retrieve kafka group coordinator");
            util::Timer::singleShot(1.0, &impl_, &KafkaConnector::Impl::rejoinGroup);
        }
    }
}

void KafkaConnector::MetadataConnection::readMetaData(impl::KafkaRawReader & reader)
{
    impl_.allBrokers_.clear();
    impl_.responsibilities_.clear();

    int32 brokerCount;
    reader >> brokerCount;
    for (int32 i = 0 ; i < brokerCount ; ++i) {
        int32 nodeId;
        impl::KafkaString host;
        int32 port;
        reader >> nodeId >> host >> port;
        impl_.allBrokers_[nodeId] = KafkaConnector::Address(ByteArray(host), (uint16)port);
    }

    int32 topicCount;
    reader >> topicCount;
    for (int32 i = 0 ; i < topicCount ; ++i) {

        int16 topicErrorCode;
        impl::KafkaString topic;
        reader >> topicErrorCode >> topic;

        int32 partitionCount;
        reader >> partitionCount;
        for (int32 i = 0 ; i < partitionCount ; ++i) {

            int16 partitionErrorCode;
            int32 partitionId;
            int32 leader;
            reader >> partitionErrorCode >> partitionId >> leader;
            if (topicErrorCode == KafkaConnector::NoError && partitionErrorCode == KafkaConnector::NoError && !topic.startsWith("__")) {
                impl_.responsibilities_[topic][partitionId].id = leader;
            }

            int32 replicaCount;
            reader >> replicaCount;
            for (int32 i = 0 ; i < replicaCount ; ++i) {
                int32 replica;
                reader >> replica;
            }

            int32 isrCount;
            reader >> isrCount;
            for (int32 i = 0 ; i < isrCount ; ++i) {
                int32 isr;
                reader >> isr;
            }
        }
    }

    close();
}

void KafkaConnector::MetadataConnection::readGroupCoordinator(impl::KafkaRawReader & reader)
{
    int16 errorCode;
    int32 coordinatorId;
    impl::KafkaString coordinatorHost;
    int32 coordinatorPort;
    reader >> errorCode >> coordinatorId >> coordinatorHost >> coordinatorPort;

    if (errorCode != KafkaConnector::NoError) {
        logInfo("got error %1 in group coordinator request", errorCode);
        close();
        return;
    }

    logInfo("got group coordinator at ip: %1, port: %2", (ByteArray)coordinatorHost, coordinatorPort);

    TCPConnData * data = impl_.net_.openConnection(coordinatorHost, coordinatorPort);
    if (!data) {
        logWarn("could not connect to group coordinator");
    } else {
        impl_.groupConnection_ = new KafkaConnector::GroupConnection(data, impl_);
        impl_.doJoin();
    }

    close();
}

} // namespace
