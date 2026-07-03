/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/string.h>
#include <cflib/base/types.h>

namespace cflib::base {

class Url
{
public:
    Url() : port_(-1) {}

    Url(const String & url) : port_(-1) {
        parse(url.str());
    }

    Url(const char * url) : port_(-1) {
        if (url) parse(std::string(url));
    }

    String scheme() const { return scheme_; }
    String host()   const { return host_; }
    int      port()   const { return port_; }
    String path()   const { return path_; }
    String query()  const { return query_; }
    String userInfo() const { return userInfo_; }
    bool     isValid() const { return !host_.isEmpty(); }
    bool     hasQuery() const { return !query_.isEmpty(); }

private:
    void parse(const std::string & url) {
        size_t pos = 0;

        // scheme
        size_t schemeEnd = url.find("://");
        if (schemeEnd != std::string::npos) {
            scheme_ = String(url.substr(0, schemeEnd));
            pos = schemeEnd + 3;
        }

        // userinfo@host:port
        size_t pathStart = url.find('/', pos);
        size_t queryStart = url.find('?', pos);
        size_t authorityEnd = std::min(
            pathStart  != std::string::npos ? pathStart  : url.size(),
            queryStart != std::string::npos ? queryStart : url.size()
        );
        std::string authority = url.substr(pos, authorityEnd - pos);

        // userinfo
        size_t atPos = authority.find('@');
        size_t hostStart = 0;
        if (atPos != std::string::npos) {
            userInfo_ = String(authority.substr(0, atPos));
            hostStart = atPos + 1;
        }

        // host:port
        std::string hostPort = authority.substr(hostStart);
        size_t colonPos;
        if (!hostPort.empty() && hostPort[0] == '[') {
            // IPv6
            size_t bracket = hostPort.find(']');
            if (bracket != std::string::npos) {
                host_ = String(hostPort.substr(1, bracket - 1));
                if (bracket + 1 < hostPort.size() && hostPort[bracket + 1] == ':')
                    port_ = std::atoi(hostPort.c_str() + bracket + 2);
            }
        } else {
            colonPos = hostPort.rfind(':');
            if (colonPos != std::string::npos) {
                host_ = String(hostPort.substr(0, colonPos));
                port_ = std::atoi(hostPort.c_str() + colonPos + 1);
            } else {
                host_ = String(hostPort);
            }
        }

        // path
        if (pathStart != std::string::npos) {
            size_t pEnd = queryStart != std::string::npos ? queryStart : url.size();
            path_ = String(url.substr(pathStart, pEnd - pathStart));
        }

        // query
        if (queryStart != std::string::npos) {
            query_ = String(url.substr(queryStart + 1));
        }
    }

    String scheme_;
    String host_;
    String path_;
    String query_;
    String userInfo_;
    int port_;
};

} // namespace
