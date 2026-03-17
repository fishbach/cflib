/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
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

    KafkaRequestWriter request(cfint16 apiKey, cfint16 apiVersion = 0, cfint32 correlationId = 1, cfuint32 expectedSize = 0);
    void close() { TCPConn::close(ReadWriteClosed, true); }
    void abort() { TCPConn::close(HardClosed, true); }

protected:
    virtual void reply(cfint32 correlationId, KafkaRawReader & reader) { CF_UNUSED(correlationId); CF_UNUSED(reader); }
    virtual void closed() {}

protected:
    void newBytesAvailable() override;
    void closed(CloseType type) override;

private:
    CFByteArray buffer_;

    friend class KafkaRequestWriter;
};

class KafkaRequestWriter : public KafkaRawWriter
{
public:
    KafkaRequestWriter(KafkaConnection & connection, cfuint32 expectedSize = 0);

    void send();

private:
    KafkaConnection & connection_;
};

}}}    // namespace
