/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
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

void KafkaConnector::GroupConnection::reply(qint32 correlationId, impl::KafkaRawReader & reader)
{
	if (correlationId == Impl::JoinGroup) {

		qint16 errorCode;
		qint32 generationId;
		impl::KafkaString groupProtocol;
		impl::KafkaString leaderId;
		impl::KafkaString ownMemberId;
		reader >> errorCode >> generationId >> groupProtocol >> leaderId >> ownMemberId;

		QMap<QByteArray, QSet<QByteArray>> memberTopics;

		qint32 memberCount;
		reader >> memberCount;
		for (qint32 i = 0 ; i < memberCount ; ++i) {
			impl::KafkaString memberId;
			qint32 metaDataSize;
			qint16 version;
			reader >> memberId >> metaDataSize >> version;

			qint32 topicCount;
			reader >> topicCount;
			for (qint32 i = 0 ; i < topicCount ; ++i) {
				impl::KafkaString topic;
				reader >> topic;

				memberTopics[memberId] << topic;
			}

			QByteArray userData;
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

		qint16 errorCode;
		reader >> errorCode;
		if (errorCode != KafkaConnector::NoError) {
			logInfo("got heartbeat error: %1", errorCode);
			impl_.rejoinGroup();
		}

	} else if (correlationId == Impl::SyncGroup) {

		QMutableMapIterator<QByteArray, QList<qint32>> it(impl_.groupTopicPartitions_);
		while (it.hasNext()) it.next().value().clear();

		qint16 errorCode;
		qint32 memberAssignmentSize;
		qint16 version;
		reader >> errorCode >> memberAssignmentSize >> version;

		qint32 topicCount;
		reader >> topicCount;
		for (qint32 i = 0 ; i < topicCount ; ++i) {
			impl::KafkaString topic;
			reader >> topic;

			QList<qint32> & partitions = impl_.groupTopicPartitions_[topic];
			qint32 partitionCount;
			reader >> partitionCount;
			for (qint32 i = 0 ; i < partitionCount ; ++i) {
				qint32 partition;
				reader >> partition;
				if (errorCode == KafkaConnector::NoError) partitions << partition;
			}
		}

		QByteArray userData;
		reader >> userData;

		impl_.main_.groupStateChanged(impl_.groupTopicPartitions_);
		impl_.joinInProgress_ = false;
		if (errorCode != KafkaConnector::NoError) close();

	} else if (correlationId == Impl::LeaveGroup) {

		qint16 errorCode;
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

}}	// namespace
