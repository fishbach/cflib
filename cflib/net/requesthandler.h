/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

namespace cflib::net {

class Request;

class RequestHandler
{
public:
    virtual ~RequestHandler() {}
    virtual void shutdown() {};

protected:
    virtual void handleRequest(const Request & request) = 0;
    friend class Request;
};

} // namespace
