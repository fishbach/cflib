/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/impl/kafkaconnection.h>
#include <cflib/net/impl/kafkaconnectorimpl.h>

namespace cflib { namespace net {

class KafkaConnector::OffsetConnection : public impl::KafkaConnection
{
public:
	OffsetConnection(TCPConnData * data, KafkaConnector::Impl & impl);

protected:
	void reply(qint32 correlationId, impl::KafkaRawReader & reader) override;
	void closed() override;

private:
	KafkaConnector::Impl & impl_;
	bool ok_;
};

}}	// namespace
