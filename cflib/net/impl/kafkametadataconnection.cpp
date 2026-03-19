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

void KafkaConnector::MetadataConnection::reply(cfint32, impl::KafkaRawReader & reader)
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
                partitionStr += ByteArray::number(partitionId);
            }
            logInfo("found topic \"%1\" (partitions: %2)", topic, partitionStr);
        }

        if (impl_.allBrokers_.empty()) {
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

    cfint32 brokerCount;
    reader >> brokerCount;
    for (cfint32 i = 0 ; i < brokerCount ; ++i) {
        cfint32 nodeId;
        impl::KafkaString host;
        cfint32 port;
        reader >> nodeId >> host >> port;
        impl_.allBrokers_[nodeId] = KafkaConnector::Address(ByteArray(host), (cfuint16)port);
    }

    cfint32 topicCount;
    reader >> topicCount;
    for (cfint32 i = 0 ; i < topicCount ; ++i) {

        cfint16 topicErrorCode;
        impl::KafkaString topic;
        reader >> topicErrorCode >> topic;

        cfint32 partitionCount;
        reader >> partitionCount;
        for (cfint32 i = 0 ; i < partitionCount ; ++i) {

            cfint16 partitionErrorCode;
            cfint32 partitionId;
            cfint32 leader;
            reader >> partitionErrorCode >> partitionId >> leader;
            if (topicErrorCode == KafkaConnector::NoError && partitionErrorCode == KafkaConnector::NoError && !topic.startsWith("__")) {
                impl_.responsibilities_[topic][partitionId].id = leader;
            }

            cfint32 replicaCount;
            reader >> replicaCount;
            for (cfint32 i = 0 ; i < replicaCount ; ++i) {
                cfint32 replica;
                reader >> replica;
            }

            cfint32 isrCount;
            reader >> isrCount;
            for (cfint32 i = 0 ; i < isrCount ; ++i) {
                cfint32 isr;
                reader >> isr;
            }
        }
    }

    close();
}

void KafkaConnector::MetadataConnection::readGroupCoordinator(impl::KafkaRawReader & reader)
{
    cfint16 errorCode;
    cfint32 coordinatorId;
    impl::KafkaString coordinatorHost;
    cfint32 coordinatorPort;
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
