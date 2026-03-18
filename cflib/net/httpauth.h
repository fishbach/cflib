/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/requesthandler.h>

#include <cflib/base/cfcontainers.h>
#include <cflib/base/cfdatetime.h>

namespace cflib { namespace net {

class HttpAuth : public RequestHandler
{
public:
    HttpAuth(const CFByteArray & name, const CFString & htpasswd = CFString());

    void addUser(const CFString & name, const CFByteArray & passwordHash);
    void reset();

protected:
    virtual void handleRequest(const Request & request);

private:
    const CFByteArray name_;
    const CFString htpasswd_;
    CFDateTime htpasswdLastMod_;
    CFMap<CFString, CFByteArray> users_;
    CFSet<CFByteArray> checkedUsers_;
};

}}    // namespace
