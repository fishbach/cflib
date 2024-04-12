/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/sig.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class TCPManager;

class HttpRequest : public util::ThreadVerify
{
    Q_DISABLE_COPY(HttpRequest)
public:
    HttpRequest(TCPManager & mgr);
    ~HttpRequest();

    void start(const QUrl & url, const QList<QByteArray> & headers,
        const QByteArray & postData = QByteArray(), const QByteArray & contentType = "application/octet-stream",
        uint timeoutMs = 10000);

    inline void start(const QUrl & url,
        const QByteArray & postData = QByteArray(), const QByteArray & contentType = "application/octet-stream",
        uint timeoutMs = 10000)
    {
        start(url, QList<QByteArray>(), postData, contentType, timeoutMs);
    }

cfsignals:
    sig<void (int status, const QByteArray & reply)> reply;

private:
    void destroy();

private:
    TCPManager & mgr_;
    class Conn;
    Conn * conn_;
};

}}    // namespace
