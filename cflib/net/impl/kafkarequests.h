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

#include <cflib/net/impl/kafkaraw.h>
#include <cflib/net/tcpconn.h>

namespace cflib { namespace net { namespace impl {

class KafkaRequestWriter;

class KafkaConnection : protected TCPConn
{
public:
	KafkaConnection(TCPConnData * data);

	KafkaRequestWriter request(qint16 apiKey, qint16 apiVersion = 0, qint32 correlationId = 1);

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
