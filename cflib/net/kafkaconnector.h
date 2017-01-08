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
	typedef QList<QByteArray> Topics;

public:
	KafkaConnector(util::ThreadVerify * other = 0);
	virtual ~KafkaConnector();

	void connect(const QByteArray & destAddress, quint16 destPort);
	void connect(const QList<Address> & cluster);

	// requiredAcks: 0 -> no response will be send / 1 -> wait for local write / -1 -> wait for all replicas
	// ackTimeoutMs: 0 -> wait for local write only / >0 -> max wait time for acks of replicas
	void produce(const QByteArray & topic, qint32 partitionId, const Messages & messages,
		quint16 requiredAcks = 1, quint32 ackTimeoutMs = 0, quint32 correlationId = 1);

	void getFirstOffset(const QByteArray & topic, qint32 partitionId, quint32 correlationId = 1);
	// last offset + 1
	void getHighwaterMarkOffset(const QByteArray & topic, qint32 partitionId, quint32 correlationId = 1);

	void fetch(const QByteArray & topic, qint32 partitionId, qint64 offset,
		quint32 maxWaitTime = 0x7FFFFFFF, quint32 minBytes = 1, quint32 maxBytes = 0x100000 /* 1mb */, quint32 correlationId = 1);

	// only one group can be joined simultaneously
	void joinGroup(const QByteArray & groupId, const Topics & topics);
	void fetch(quint32 maxWaitTime = 0x7FFFFFFF, quint32 minBytes = 1, quint32 maxBytes = 0x100000 /* 1mb */);
	void commit();	// commits last fetchResponse
	void leaveGroup();

protected:
	virtual void stateChanged(State state) { Q_UNUSED(state) }

	// offset -> is offset of first message appended to the kafka log
	virtual void produceResponse(quint32 correlationId, ErrorCode errorCode, qint64 offset) {
		Q_UNUSED(correlationId) Q_UNUSED(errorCode) Q_UNUSED(offset) }

	virtual void offsetResponse(quint32 correlationId, qint64 offset) {
		Q_UNUSED(correlationId) Q_UNUSED(offset) }

	// highwaterMarkOffset -> last offset + 1
	virtual void fetchResponse(quint32 correlationId, const Messages & messages,
		qint64 firstOffset, qint64 highwaterMarkOffset, ErrorCode errorCode) {
		Q_UNUSED(correlationId) Q_UNUSED(messages) Q_UNUSED(firstOffset) Q_UNUSED(highwaterMarkOffset) Q_UNUSED(errorCode) }

	virtual void fetchResponse(const QByteArray & topic, const Messages & messages,
		qint64 firstOffset, qint64 highwaterMarkOffset, ErrorCode errorCode) {
		Q_UNUSED(topic) Q_UNUSED(messages) Q_UNUSED(firstOffset) Q_UNUSED(highwaterMarkOffset) Q_UNUSED(errorCode) }

private:
	class MetadataConnection;
	class ProduceConnection;
	class OffsetConnection;
	class FetchConnection;
	class GroupConnection;
	class Impl;
	Impl * impl_;
};

}}	// namespace
