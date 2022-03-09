/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

#include "kafkametadataconnection.h"

#include <cflib/net/impl/kafkagroupconnection.h>
#include <cflib/util/log.h>
#include <cflib/util/timer.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

KafkaConnector::MetadataConnection::MetadataConnection(bool isMetaDataRequest, TCPConnData * data, KafkaConnector::Impl & impl) :
	KafkaConnection(data),
	impl_(impl),
	isMetaDataRequest_(isMetaDataRequest)
{
}

void KafkaConnector::MetadataConnection::reply(qint32, impl::KafkaRawReader & reader)
{
	if (isMetaDataRequest_) readMetaData        (reader);
	else                    readGroupCoordinator(reader);
}

void KafkaConnector::MetadataConnection::closed()
{
	if (isMetaDataRequest_) {
		for (qint32 nodeId : impl_.allBrokers_.keys()) {
			const KafkaConnector::Address & addr = impl_.allBrokers_[nodeId];
			logInfo("found broker %1 with ip %2 (port: %3)", nodeId, addr.first, addr.second);
		}

		for (const QByteArray & topic : impl_.responsibilities_.keys()) {
			QByteArray partitionStr;
			bool isFirst = true;
			for (qint32 partitionId : impl_.responsibilities_[topic].keys()) {
				if (isFirst) isFirst = false;
				else         partitionStr += ' ';
				partitionStr += QByteArray::number(partitionId);
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

	qint32 brokerCount;
	reader >> brokerCount;
	for (qint32 i = 0 ; i < brokerCount ; ++i) {
		qint32 nodeId;
		impl::KafkaString host;
		qint32 port;
		reader >> nodeId >> host >> port;
		impl_.allBrokers_[nodeId] = qMakePair(host, port);
	}

	qint32 topicCount;
	reader >> topicCount;
	for (qint32 i = 0 ; i < topicCount ; ++i) {

		qint16 topicErrorCode;
		impl::KafkaString topic;
		reader >> topicErrorCode >> topic;

		qint32 partitionCount;
		reader >> partitionCount;
		for (qint32 i = 0 ; i < partitionCount ; ++i) {

			qint16 partitionErrorCode;
			qint32 partitionId;
			qint32 leader;
			reader >> partitionErrorCode >> partitionId >> leader;
			if (topicErrorCode == KafkaConnector::NoError && partitionErrorCode == KafkaConnector::NoError && !topic.startsWith("__")) {
				impl_.responsibilities_[topic][partitionId].id = leader;
			}

			qint32 replicaCount;
			reader >> replicaCount;
			for (qint32 i = 0 ; i < replicaCount ; ++i) {
				qint32 replica;
				reader >> replica;
			}

			qint32 isrCount;
			reader >> isrCount;
			for (qint32 i = 0 ; i < isrCount ; ++i) {
				qint32 isr;
				reader >> isr;
			}
		}
	}

	close();
}

void KafkaConnector::MetadataConnection::readGroupCoordinator(impl::KafkaRawReader & reader)
{
	qint16 errorCode;
	qint32 coordinatorId;
	impl::KafkaString coordinatorHost;
	qint32 coordinatorPort;
	reader >> errorCode >> coordinatorId >> coordinatorHost >> coordinatorPort;

	if (errorCode != KafkaConnector::NoError) {
		logInfo("got error %1 in group coordinator request", errorCode);
		close();
		return;
	}

	logInfo("got group coordinator at ip: %1, port: %2", (QByteArray)coordinatorHost, coordinatorPort);

	TCPConnData * data = impl_.net_.openConnection(coordinatorHost, coordinatorPort);
	if (!data) {
		logWarn("could not connect to group coordinator");
	} else {
		impl_.groupConnection_ = new KafkaConnector::GroupConnection(data, impl_);
		impl_.doJoin();
	}

	close();
}

}}	// namespace
