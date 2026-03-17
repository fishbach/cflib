/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfurl.h>
#include <cflib/util/sig.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class TCPManager;

class HttpRequest : public util::ThreadVerify
{
    CF_DISABLE_COPY(HttpRequest)
public:
    HttpRequest(TCPManager & mgr);
    ~HttpRequest();

    void start(const CFUrl & url, const CFList<CFByteArray> & headers,
        const CFByteArray & postData = CFByteArray(), const CFByteArray & contentType = "application/octet-stream",
        uint timeoutMs = 10000);

    inline void start(const CFUrl & url,
        const CFByteArray & postData = CFByteArray(), const CFByteArray & contentType = "application/octet-stream",
        uint timeoutMs = 10000)
    {
        start(url, CFList<CFByteArray>(), postData, contentType, timeoutMs);
    }

cfsignals:
    sig<void (int status, const CFByteArray & reply)> reply;

private:
    void destroy();

private:
    TCPManager & mgr_;
    class Conn;
    Conn * conn_;
};

}}    // namespace
