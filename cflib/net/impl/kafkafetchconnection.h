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
#include <cflib/net/impl/kafkaconnectorimpl.h>

namespace cflib { namespace net {

class KafkaConnector::FetchConnection : public impl::KafkaConnection
{
public:
	FetchConnection(TCPConnData * data, KafkaConnector::Impl & impl);

protected:
	void reply(qint32 correlationId, impl::KafkaRawReader & reader) override;
	void closed() override;

private:
	KafkaConnector::Impl & impl_;
};

}}	// namespace
