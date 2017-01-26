/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#include "kafkagroupconnection.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

KafkaConnector::GroupConnection::GroupConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
	KafkaConnection(data),
	impl_(impl)
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
			impl_.groupAssignment_.clear();
			close();
		} else {
			impl_.generationId_ = generationId;
			impl_.groupMemberId_ = ownMemberId;
			impl_.computeGroupAssignment(groupProtocol, memberTopics);
			impl_.doSync();
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
				partitions << partition;
			}
		}

		QByteArray userData;
		reader >> userData;

		impl_.main_.groupStateChanged(impl_.groupTopicPartitions_);
		impl_.joinInProgress_ = false;
	}
}

void KafkaConnector::GroupConnection::closed()
{
	impl_.groupHeartbeatTimer_.stop();
	impl_.groupConnection_ = 0;
	impl_.rejoinGroup();
}

}}	// namespace
