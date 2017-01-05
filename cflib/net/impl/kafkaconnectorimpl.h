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

#pragma once

#include <cflib/net/kafkaconnector.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class KafkaConnector::Impl : public util::ThreadVerify
{
public:
	Impl(KafkaConnector & main);
	Impl(KafkaConnector & parent, util::ThreadVerify * other);
	~Impl();

	void setState(KafkaConnector::State newState);
	void connect(const QList<KafkaConnector::Address> & cluster);
	void fetchMetaData();

	void produce(const QByteArray & topic, qint32 partitionId, const KafkaConnector::Messages & messages,
		quint16 requiredAcks, quint32 ackTimeoutMs, quint32 correlationId);
	void fetch(const QByteArray & topic, qint32 partitionId, qint64 offset,
		quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes, quint32 correlationId);

	void joinGroup(const QByteArray & groupName, const QList<QByteArray> & topics);
	void fetch(quint32 maxWaitTime, quint32 minBytes, quint32 maxBytes);
	void commit();
	void leaveGroup();

public:
	KafkaConnector & main_;
	TCPManager net_;
	QList<KafkaConnector::Address> cluster_;
	int clusterId_;
	QHash<qint32 /* nodeId */, KafkaConnector::Address> allBrokers_;
	struct NodeId { qint32 id; NodeId() : id(-1) {} };
	QMap<QByteArray /* topic */, QMap<qint32 /* partitionId */, NodeId>> responsibilities_;
	KafkaConnector::State currentState_;
	QHash<qint32 /* nodeId */, KafkaConnector::ProduceConnection *> produceConnections_;
	QHash<qint32 /* nodeId */, KafkaConnector::FetchConnection   *> fetchConnections_;
};

}}	// namespace
