/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/impl/kafkaconnection.h>
#include <cflib/net/impl/kafkaconnectorimpl.h>

namespace cflib { namespace net {

class KafkaConnector::MetadataConnection : public impl::KafkaConnection
{
public:
    // isMetaDataRequest == false -> group coordinator request
    MetadataConnection(bool isMetaDataRequest, TCPConnData * data, KafkaConnector::Impl & impl);

protected:
    void reply(qint32, impl::KafkaRawReader & reader) override;
    void closed() override;

private:
    void readMetaData(impl::KafkaRawReader & reader);
    void readGroupCoordinator(impl::KafkaRawReader & reader);

private:
    KafkaConnector::Impl & impl_;
    const bool isMetaDataRequest_;
};

}}    // namespace
