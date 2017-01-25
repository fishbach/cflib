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

#include <cflib/net/impl/kafkaconnection.h>
#include <cflib/net/impl/kafkaconnectorimpl.h>

namespace cflib { namespace net {

class KafkaConnector::GroupConnection : public impl::KafkaConnection
{
public:
	GroupConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
		KafkaConnection(data),
		impl_(impl)
	{
	}

protected:
	void reply(qint32 correlationId, impl::KafkaRawReader & reader) override
	{
		if (correlationId == 1) {
			// join

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
				impl_.groupMemberId_ = "";
				impl_.groupAssignment_.clear();
				close();
			} else {
				impl_.generationId_ = generationId;
				impl_.groupMemberId_ = ownMemberId;
				impl_.computeGroupAssignment(groupProtocol, memberTopics);
				impl_.doSync();
			}

		} else if (correlationId == 2) {
			// heartbeat

			qint16 errorCode;
			reader >> errorCode;

		} else if (correlationId == 3) {
			// sync

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
				partitions.clear();

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
		}
	}

	void closed() override
	{
		impl_.groupConnection_ = 0;
		impl_.joinGroup(impl_.groupId_, impl_.groupTopicPartitions_.keys(), impl_.preferredStrategy_);
	}

private:
	KafkaConnector::Impl & impl_;
};

}}	// namespace
