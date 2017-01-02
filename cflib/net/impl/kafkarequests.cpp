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

#include "kafkarequests.h"

#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net { namespace impl {

namespace {

const KafkaString kafkaClientName = "cflib";

}

KafkaConnection::KafkaConnection(TCPConnData * data) :
	TCPConn(data)
{
	setNoDelay(true);
	startReadWatcher();
}

KafkaRequestWriter KafkaConnection::request(qint16 apiKey, qint16 apiVersion, qint32 correlationId, quint32 expectedSize)
{
	KafkaRequestWriter rv(*this, kafkaClientName.size() + 10 + expectedSize);
	rv
		<< apiKey
		<< apiVersion
		<< correlationId
		<< kafkaClientName;
	return rv;
}

void KafkaConnection::newBytesAvailable()
{
	buffer_ += read();

	// enough bytes for size?
	while (buffer_.size() >= 4) {
		cflib::net::impl::KafkaRawReader reader(buffer_);

		// read size
		qint32 size;
		reader >> size;
		if (size <= 0) {
			logWarn("funny size of reply: %1", size);
			close(HardClosed, true);
			buffer_.clear();
			return;
		}

		// enough bytes?
		if (buffer_.size() < size + 4) break;

		qint32 correlationId;
		reader >> correlationId;

		reply(correlationId, reader);

		buffer_.remove(0, size + 4);
	}

	startReadWatcher();
}

void KafkaConnection::closed(TCPConn::CloseType)
{
	closed();
	util::deleteNext(this);
}

KafkaRequestWriter::KafkaRequestWriter(KafkaConnection & connection, quint32 expectedSize) :
	KafkaRawWriter(expectedSize),
	connection_(connection)
{
}

void KafkaRequestWriter::send()
{
	connection_.write(getData());
}

}}}	// namespace
