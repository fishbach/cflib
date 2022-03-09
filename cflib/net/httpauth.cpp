/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "httpauth.h"

#include <cflib/crypt/util.h>
#include <cflib/net/request.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

HttpAuth::HttpAuth(const QByteArray & name) :
	authRe_("^Basic\\s+([A-Za-z0-9+/]+=*)$"),
	name_(name)
{
}

void HttpAuth::addUser(const QByteArray & name, const QByteArray & passwordHash)
{
	users_[name] = passwordHash;
}

void HttpAuth::reset()
{
	users_.clear();
	checkedUsers_.clear();
}

void HttpAuth::handleRequest(const Request & request)
{
	const QByteArray auth = request.getHeader("authorization");
	if (checkedUsers_.contains(auth)) return;
	const QRegularExpressionMatch match = authRe_.match(auth);
	if (match.hasMatch()) {
		QList<QByteArray> userPass = QByteArray::fromBase64(match.captured(1).toUtf8()).split(':');
		if (userPass.size() == 2) {
			const QByteArray hash = users_.value(userPass[0]);
			if (!hash.isEmpty() && crypt::checkPassword(userPass[1], hash)) {
				checkedUsers_ << auth;
				return;
			}
		}
	}

	request.sendRaw(
		"HTTP/1.1 401 Not Authorized\r\n"
		"WWW-Authenticate: Basic realm=\"" << name_ << "\"\r\n"
		<< request.defaultHeaders() <<
		"Content-Type: text/html; charset=utf-8\r\n",

		"<html>\r\n"
		"<head><title>401 - Not Authorized</title></head>\r\n"
		"<body>\r\n"
		"<h1>401 - Not Authorized</h1>\r\n"
		"</body>\r\n"
		"</html>\r\n",
		false);
}

}}	// namespace
