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

#include "kafkaconnector.h"

#include <cflib/net/impl/kafkarequests.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>
#include <cflib/util/timer.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

class KafkaConnector::Impl : public util::ThreadVerify
{
public:
	Impl(KafkaConnector & main) :
		ThreadVerify("KafkaConnector", ThreadVerify::Net),
		main_(main),
		net_(0, this),
		clusterId_(0),
		currentState_(KafkaConnector::Idle)
	{
	}

	Impl(KafkaConnector & parent, util::ThreadVerify * other) :
		ThreadVerify(other),
		main_(parent),
		net_(0, this),
		clusterId_(0),
		currentState_(KafkaConnector::Idle)
	{
	}

	~Impl()
	{
		logFunctionTrace
		stopVerifyThread();
	}

	void connect(const QList<KafkaConnector::Address> & cluster)
	{
		if (!verifyThreadCall(&Impl::connect, cluster)) return;

		if (!cluster_.isEmpty()) {
			/// TODO abort all
		}

		cluster_ = cluster;
		clusterId_ = 0;

		fetchMetaData();
	}

private:
	void fetchMetaData();
	void refetchMetaData();
	void setState(KafkaConnector::State newState);

private:
	KafkaConnector & main_;
	cflib::net::TCPManager net_;
	QList<KafkaConnector::Address> cluster_;
	int clusterId_;
	QHash<qint32 /* nodeId */, KafkaConnector::Address> allBrokers_;
	QMap<QByteArray /* topic */, QMap<qint32 /* partitionId */, qint32 /* nodeId */>> responsibilities_;
	KafkaConnector::State currentState_;

	friend class MetadataConnection;
};

// ============================================================================
// ============================================================================

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
		qint32 brokerCount;
		reader >> brokerCount;
		for (qint32 i = 0 ; i < brokerCount ; ++i) {
			qint32 nodeId;
			cflib::net::impl::KafkaString host;
			qint32 port;
			reader >> nodeId >> host >> port;
			impl_.allBrokers_[nodeId] = qMakePair(host, port);
		}

		qint32 topicCount;
		reader >> topicCount;
		for (qint32 i = 0 ; i < topicCount ; ++i) {

			qint16 topicErrorCode;
			cflib::net::impl::KafkaString topic;
			reader >> topicErrorCode >> topic;

			qint32 partitionCount;
			reader >> partitionCount;
			for (qint32 i = 0 ; i < partitionCount ; ++i) {

				qint16 partitionErrorCode;
				qint32 partitionId;
				qint32 leader;
				reader >> partitionErrorCode >> partitionId >> leader;
				if (topicErrorCode == 0 && partitionErrorCode == 0 && !topic.startsWith("__")) {
					impl_.responsibilities_[topic][partitionId] = leader;
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
			logInfo("found broker with ip %1 (port: %2)", addr.first, addr.second);
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
			impl_.refetchMetaData();
		} else {
			impl_.setState(Ready);
		}
	}

private:
	KafkaConnector::Impl & impl_;
};

// ============================================================================
// ============================================================================

void KafkaConnector::Impl::fetchMetaData()
{
	if (cluster_.isEmpty()) {
		setState(Idle);
		return;
	}

	setState(Connecting);

	const int startId = clusterId_;
	do {
		const KafkaConnector::Address & addr = cluster_[clusterId_];
		if (++clusterId_ >= cluster_.size()) clusterId_ = 0;
		TCPConnData * data = net_.openConnection(addr.first, addr.second);
		if (data) {
			new KafkaConnector::MetadataConnection(data, *this);
			return;
		}
	} while (clusterId_ != startId);

	refetchMetaData();
}

void KafkaConnector::Impl::refetchMetaData()
{
	logWarn("could not connect to broker");
	util::Timer::singleShot(10.0, this, &Impl::fetchMetaData);
}

void KafkaConnector::Impl::setState(KafkaConnector::State newState)
{
	if (currentState_ == newState) return;

	currentState_ = newState;
	main_.stateChanged(newState);
}

// ============================================================================
// ============================================================================

KafkaConnector::KafkaConnector(util::ThreadVerify * other) :
	impl_(other ?
		new Impl(*this, other) :
		new Impl(*this))
{
}

KafkaConnector::~KafkaConnector()
{
	delete impl_;
}

void KafkaConnector::connect(const QByteArray & destAddress, quint16 destPort)
{
	connect(QList<KafkaConnector::Address>() << qMakePair(destAddress, destPort));
}

void KafkaConnector::connect(const QList<KafkaConnector::Address> & cluster)
{
	impl_->connect(cluster);
}

void KafkaConnector::produce()
{

}

}}	// namespace
