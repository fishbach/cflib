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

#include <QtCore>

namespace cflib { namespace util  { class ThreadVerify; }}

namespace cflib { namespace net {

class KafkaConnector
{
	Q_DISABLE_COPY(KafkaConnector)
public:
	enum State
	{
		Error = -1,
		Idle = 1,
		Connecting,
		Ready
	};

	enum ErrorCode
	{
		NoError = 0,
		Unknown = -1,
		OffsetOutOfRange = 1,
		InvalidMessage = 2,
		CorruptMessage = 2,
		UnknownTopicOrPartition = 3,
		InvalidMessageSize = 4,
		LeaderNotAvailable = 5,
		NotLeaderForPartition = 6,
		RequestTimedOut = 7,
		BrokerNotAvailable = 8,
		ReplicaNotAvailable = 9,
		MessageSizeTooLarge = 10,
		StaleControllerEpochCode = 11,
		OffsetMetadataTooLargeCode = 12,
		GroupLoadInProgressCode	= 14,
		GroupCoordinatorNotAvailableCode = 15,
		NotCoordinatorForGroupCode = 16,
		InvalidTopicCode = 17,
		RecordListTooLargeCode = 18,
		NotEnoughReplicasCode = 19,
		NotEnoughReplicasAfterAppendCode = 20,
		InvalidRequiredAcksCode = 21,
		IllegalGenerationCode = 22,
		InconsistentGroupProtocolCode = 23,
		InvalidGroupIdCode = 24,
		UnknownMemberIdCode = 25,
		InvalidSessionTimeoutCode = 26,
		RebalanceInProgressCode = 27,
		InvalidCommitOffsetSizeCode = 28,
		TopicAuthorizationFailedCode = 29,
		GroupAuthorizationFailedCode = 30,
		ClusterAuthorizationFailedCode = 31
	};

	typedef QPair<QByteArray /* ip */, quint16 /* port */> Address;

	typedef QPair<QByteArray /* key */, QByteArray /* value */> Message;
	typedef QVector<Message> Messages;

public:
	KafkaConnector(util::ThreadVerify * other = 0);
	virtual ~KafkaConnector();

	void connect(const QByteArray & destAddress, quint16 destPort);
	void connect(const QList<Address> & cluster);

	void produce(const QByteArray & topic, qint32 partitionId, const Messages & messages,
		quint16 requiredAcks = 1, quint32 ackTimeoutMs = 0, quint32 correlationId = 1);

protected:
	virtual void stateChanged(State state) { Q_UNUSED(state) }

	virtual void produceReply(quint32 correlationId, ErrorCode errorCode, qint64 offset) { Q_UNUSED(correlationId) Q_UNUSED(errorCode) Q_UNUSED(offset) }

private:
	class MetadataConnection;
	class ProduceConnection;
	class Impl;
	Impl * impl_;
};

}}	// namespace
