/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "dns.h"

#include <cflib/util/log.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

namespace {

const QRegularExpression ipRe("^(?:\\d+\\.\\d+\\.\\d+\\.\\d+|[:0-9A-Fa-f]+)$");

}

QList<QByteArray> getIPFromDNS(const QByteArray & name, bool preferIPv6)
{
	if (ipRe.match(name).hasMatch()) {
		logTrace("getIPFromDNS(\"%1\", %2) -> %1", name, preferIPv6);
		return QList<QByteArray>() << name;
	}

	struct addrinfo * res;
	int err = getaddrinfo(name.constData(), 0, 0, &res);
	if (err != 0) {
		logWarn("getaddrinfo failed with error: %1", err);
		return QList<QByteArray>();
	}

	QSet<QByteArray> ipv4;
	QSet<QByteArray> ipv6;
	for ( ; res ; res = res->ai_next) {
		char ip[40];
		if (res->ai_family == AF_INET) {
			inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, ip, sizeof(ip));
			ipv4 << ip;
		} else if (res->ai_family == AF_INET6) {
			inet_ntop(AF_INET6, &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, ip, sizeof(ip));
			ipv6 << ip;
		}
	}

	freeaddrinfo(res);

	QList<QByteArray> rv = ipv4.isEmpty() || (preferIPv6 && !ipv6.isEmpty()) ? ipv6.toList() : ipv4.toList();
	qSort(rv);
	logTrace("getIPFromDNS(\"%1\", %2) -> %3", name, preferIPv6, rv.join(' '));
	return rv;
}

}}	// namespace
