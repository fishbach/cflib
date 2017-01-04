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

namespace cflib { namespace net {

class KafkaConnector::MetadataConnection : public impl::KafkaConnection
{
public:
	MetadataConnection(TCPConnData * data, KafkaConnector::Impl & impl) :
		KafkaConnection(data),
		impl_(impl)
	{
		impl::KafkaRequestWriter req = request(3);
		req << (qint32)0;	// no topic specified -> get all
		req.send();
	}

protected:
	void reply(qint32, impl::KafkaRawReader & reader) override
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

		close(ReadWriteClosed, true);
	}

	void closed() override
	{
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
			util::Timer::singleShot(1.0, &impl_, &Impl::fetchMetaData);
		} else {
			impl_.setState(Ready);
		}
	}

private:
	KafkaConnector::Impl & impl_;
};

}}	// namespace
