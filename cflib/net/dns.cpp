/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "dns.h"

#include <cflib/base.h>
#include <cflib/util/log.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

USE_LOG(LogCat::Network)

namespace cflib::net {

namespace {

const Regex ipRe("^(?:\\d+\\.\\d+\\.\\d+\\.\\d+|[:0-9A-Fa-f]+)$");

}

List<ByteArray> getIPFromDNS(const ByteArray & name, bool preferIPv6)
{
    if (ipRe.match(name)) {
        logTrace("getIPFromDNS(\"%1\", %2) -> %1", name, preferIPv6);
        List<ByteArray> rv;
        rv.push_back(name);
        return rv;
    }

    struct addrinfo * res;
    int err = getaddrinfo(name.toStdString().c_str(), 0, 0, &res);
    if (err != 0) {
        logWarn("getaddrinfo failed with error: %1", err);
        return List<ByteArray>();
    }

    Set<ByteArray> ipv4;
    Set<ByteArray> ipv6;
    for ( ; res ; res = res->ai_next) {
        char ip[40];
        if (res->ai_family == AF_INET) {
            inet_ntop(AF_INET, &((struct sockaddr_in *)(void *)res->ai_addr)->sin_addr, ip, sizeof(ip));
            ipv4.insert(ByteArray(ip));
        } else if (res->ai_family == AF_INET6) {
            inet_ntop(AF_INET6, &((struct sockaddr_in6 *)(void *)res->ai_addr)->sin6_addr, ip, sizeof(ip));
            ipv6.insert(ByteArray(ip));
        }
    }

    freeaddrinfo(res);

    List<ByteArray> rv = ipv4.isEmpty() || (preferIPv6 && !ipv6.isEmpty()) ? ipv6.toList() : ipv4.toList();
    std::sort(rv.begin(), rv.end());
    logTrace("getIPFromDNS(\"%1\", %2) -> %3", name, preferIPv6, rv.join(' '));
    return rv;
}

} // namespace
