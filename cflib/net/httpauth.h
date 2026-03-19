/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/requesthandler.h>

#include <cflib/base.h>

namespace cflib::net {

class HttpAuth : public RequestHandler
{
public:
    HttpAuth(const ByteArray & name, const String & htpasswd = String());

    void addUser(const String & name, const ByteArray & passwordHash);
    void reset();

protected:
    virtual void handleRequest(const Request & request);

private:
    const ByteArray name_;
    const String htpasswd_;
    CFDateTime htpasswdLastMod_;
    CFMap<String, ByteArray> users_;
    CFSet<ByteArray> checkedUsers_;
};

} // namespace
