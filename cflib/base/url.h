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
        if (url) parse(std::string_view(url));
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
    void parse(std::string_view url) {
        constexpr size_t npos = std::string_view::npos;
        size_t pos = 0;

        // scheme
        size_t schemeEnd = url.find("://");
        if (schemeEnd != npos) {
            scheme_ = String(url.substr(0, schemeEnd));
            pos = schemeEnd + 3;
        }

        // userinfo@host:port
        size_t pathStart = url.find('/', pos);
        size_t queryStart = url.find('?', pos);
        size_t authorityEnd = std::min(
            pathStart  != npos ? pathStart  : url.size(),
            queryStart != npos ? queryStart : url.size()
        );
        std::string_view authority = url.substr(pos, authorityEnd - pos);

        // userinfo
        size_t atPos = authority.find('@');
        size_t hostStart = 0;
        if (atPos != npos) {
            userInfo_ = String(authority.substr(0, atPos));
            hostStart = atPos + 1;
        }

        // host:port
        std::string_view hostPort = authority.substr(hostStart);
        size_t colonPos;
        if (!hostPort.empty() && hostPort[0] == '[') {
            // IPv6
            size_t bracket = hostPort.find(']');
            if (bracket != npos) {
                host_ = String(hostPort.substr(1, bracket - 1));
                if (bracket + 1 < hostPort.size() && hostPort[bracket + 1] == ':')
                    port_ = std::atoi(hostPort.data() + bracket + 2);
            }
        } else {
            colonPos = hostPort.rfind(':');
            if (colonPos != npos) {
                host_ = String(hostPort.substr(0, colonPos));
                port_ = std::atoi(hostPort.data() + colonPos + 1);
            } else {
                host_ = String(hostPort);
            }
        }

        // path
        if (pathStart != npos) {
            size_t pEnd = queryStart != npos ? queryStart : url.size();
            path_ = String(url.substr(pathStart, pEnd - pathStart));
        }

        // query
        if (queryStart != npos) {
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
