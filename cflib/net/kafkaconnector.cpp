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

#include <cflib/net/impl/kafkaraw.h>
#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

class KafkaConnector::Connection : public TCPConn
{
public:
	Connection(TCPConnData * data, KafkaConnector::Impl & parent) :
		TCPConn(data),
		parent_(parent)
	{
		cflib::net::impl::KafkaRequest req(3);
		req << (qint32)0;

		qDebug() << "S: " << req.getData().toHex();

		write(req.getData());

		startReadWatcher();
	}

protected:
	void newBytesAvailable() override
	{
		buffer_ += read();
		qDebug() << "R: " << buffer_.toHex();

		while (buffer_.size() >= 4) {
			cflib::net::impl::KafkaRawReader reader(buffer_);
			qint32 size;
			reader >> size;
			if (buffer_.size() < size + 4) break;

			qint32 correlationId;
			reader >> correlationId;

			qint32 brokerCount;
			reader >> brokerCount;
			for (qint32 i = 0 ; i < brokerCount ; ++i) {
				qint32 nodeId;
				cflib::net::impl::KafkaString host;
				qint32 port;
				reader >> nodeId >> host >> port;
				qDebug() << "broker: " << nodeId << " / " << host << ":" << port;
			}

			qint32 topicCount;
			reader >> topicCount;
			for (qint32 i = 0 ; i < topicCount ; ++i) {

				qint16 topicErrorCode;
				cflib::net::impl::KafkaString topic;
				reader >> topicErrorCode >> topic;
				qDebug() << "topic: " << topicErrorCode << " / " << topic;

				qint32 partitionCount;
				reader >> partitionCount;
				for (qint32 i = 0 ; i < partitionCount ; ++i) {

					qint16 partitionErrorCode;
					qint32 partitionId;
					qint32 leader;
					reader >> partitionErrorCode >> partitionId >> leader;
					qDebug() << "partition: " << partitionErrorCode << " / " << partitionId << " / " << leader;

					qint32 replicaCount;
					reader >> replicaCount;
					for (qint32 i = 0 ; i < replicaCount ; ++i) {
						qint32 replica;
						reader >> replica;
						qDebug() << "replica: " << replica;
					}

					qint32 isrCount;
					reader >> isrCount;
					for (qint32 i = 0 ; i < isrCount ; ++i) {
						qint32 isr;
						reader >> isr;
						qDebug() << "isr: " << isr;
					}
				}
			}

			buffer_.remove(0, size + 4);
		}

		qDebug() << "rest size: " << buffer_.size();

		startReadWatcher();
	}

	void closed(CloseType type) override;

private:
	KafkaConnector::Impl & parent_;
	QByteArray buffer_;
};


class KafkaConnector::Impl : public util::ThreadVerify
{
public:
	Impl(KafkaConnector & parent) :
		ThreadVerify("KafkaConnector", ThreadVerify::Net),
		parent_(parent),
		net_(0, this),
		clusterId_(0)
	{
	}

	Impl(KafkaConnector & parent, util::ThreadVerify * other) :
		ThreadVerify(other),
		parent_(parent),
		net_(0, this),
		clusterId_(0)
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

		parent_.stateChanged(cluster_.isEmpty() ? Idle : Connecting);
	}

private:
	void fetchMetaData()
	{
		if (cluster_.isEmpty()) return;

		const Address & addr = cluster_[clusterId_];
		if (++clusterId_ >= cluster_.size()) clusterId_ = 0;

		TCPConnData * data = net_.openConnection(addr.first, addr.second);

		if (data) new Connection(data, *this);
	}

private:
	KafkaConnector & parent_;
	cflib::net::TCPManager net_;
	QList<KafkaConnector::Address> cluster_;
	int clusterId_;

	friend class Connection;
};


void KafkaConnector::Connection::closed(TCPConn::CloseType type)
{
	qDebug() << "closed (" << (int)type << ")";

	parent_.parent_.stateChanged(parent_.cluster_.isEmpty() ? Idle : Connecting);

	util::deleteNext(this);
}


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
