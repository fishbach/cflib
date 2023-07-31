/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/impl/kafkaraw.h>
#include <cflib/net/tcpconn.h>

namespace cflib { namespace net { namespace impl {

class KafkaRequestWriter;

class KafkaConnection : protected TCPConn
{
public:
	KafkaConnection(TCPConnData * data);

	KafkaRequestWriter request(qint16 apiKey, qint16 apiVersion = 0, qint32 correlationId = 1, quint32 expectedSize = 0);
	void close() { TCPConn::close(ReadWriteClosed, true); }
	void abort() { TCPConn::close(HardClosed, true); }

protected:
	virtual void reply(qint32 correlationId, KafkaRawReader & reader) { Q_UNUSED(correlationId) Q_UNUSED(reader) }
	virtual void closed() {}

protected:
	void newBytesAvailable() override;
	void closed(CloseType type) override;

private:
	QByteArray buffer_;

	friend class KafkaRequestWriter;
};

class KafkaRequestWriter : public KafkaRawWriter
{
public:
	KafkaRequestWriter(KafkaConnection & connection, quint32 expectedSize = 0);

	void send();

private:
	KafkaConnection & connection_;
};

}}}	// namespace
