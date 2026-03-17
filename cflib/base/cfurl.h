/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfstring.h>
#include <cflib/base/types.h>

class CFUrl
{
public:
    CFUrl() : port_(-1) {}

    explicit CFUrl(const CFString & url) : port_(-1) {
        parse(url.str());
    }

    explicit CFUrl(const char * url) : port_(-1) {
        if (url) parse(std::string(url));
    }

    CFString scheme() const { return scheme_; }
    CFString host()   const { return host_; }
    int      port()   const { return port_; }
    CFString path()   const { return path_; }
    CFString query()  const { return query_; }
    CFString userInfo() const { return userInfo_; }
    bool     isValid() const { return !host_.isEmpty(); }
    bool     hasQuery() const { return !query_.isEmpty(); }

private:
    void parse(const std::string & url) {
        cfsize_t pos = 0;

        // scheme
        cfsize_t schemeEnd = url.find("://");
        if (schemeEnd != std::string::npos) {
            scheme_ = CFString(url.substr(0, schemeEnd));
            pos = schemeEnd + 3;
        }

        // userinfo@host:port
        cfsize_t pathStart = url.find('/', pos);
        cfsize_t queryStart = url.find('?', pos);
        cfsize_t authorityEnd = std::min(
            pathStart  != std::string::npos ? pathStart  : url.size(),
            queryStart != std::string::npos ? queryStart : url.size()
        );
        std::string authority = url.substr(pos, authorityEnd - pos);

        // userinfo
        cfsize_t atPos = authority.find('@');
        cfsize_t hostStart = 0;
        if (atPos != std::string::npos) {
            userInfo_ = CFString(authority.substr(0, atPos));
            hostStart = atPos + 1;
        }

        // host:port
        std::string hostPort = authority.substr(hostStart);
        cfsize_t colonPos;
        if (!hostPort.empty() && hostPort[0] == '[') {
            // IPv6
            cfsize_t bracket = hostPort.find(']');
            if (bracket != std::string::npos) {
                host_ = CFString(hostPort.substr(1, bracket - 1));
                if (bracket + 1 < hostPort.size() && hostPort[bracket + 1] == ':')
                    port_ = std::atoi(hostPort.c_str() + bracket + 2);
            }
        } else {
            colonPos = hostPort.rfind(':');
            if (colonPos != std::string::npos) {
                host_ = CFString(hostPort.substr(0, colonPos));
                port_ = std::atoi(hostPort.c_str() + colonPos + 1);
            } else {
                host_ = CFString(hostPort);
            }
        }

        // path
        if (pathStart != std::string::npos) {
            cfsize_t pEnd = queryStart != std::string::npos ? queryStart : url.size();
            path_ = CFString(url.substr(pathStart, pEnd - pathStart));
        }

        // query
        if (queryStart != std::string::npos) {
            query_ = CFString(url.substr(queryStart + 1));
        }
    }

    CFString scheme_;
    CFString host_;
    CFString path_;
    CFString query_;
    CFString userInfo_;
    int port_;
};
