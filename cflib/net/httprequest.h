/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/util/sig.h>
#include <cflib/util/threadverify.h>

namespace cflib::net {

class TCPManager;

class HttpRequest : public util::ThreadVerify
{
    CF_DISABLE_COPY(HttpRequest)
public:
    HttpRequest(TCPManager & mgr);
    ~HttpRequest();

    void start(const Url & url, const List<ByteArray> & headers,
        const ByteArray & postData = ByteArray(), const ByteArray & contentType = "application/octet-stream",
        uint timeoutMs = 10000);

    inline void start(const Url & url,
        const ByteArray & postData = ByteArray(), const ByteArray & contentType = "application/octet-stream",
        uint timeoutMs = 10000)
    {
        start(url, List<ByteArray>(), postData, contentType, timeoutMs);
    }

cfsignals:
    sig<void (int status, const ByteArray & reply)> reply;

private:
    void destroy();

private:
    TCPManager & mgr_;
    class Conn;
    Conn * conn_;
};

} // namespace
