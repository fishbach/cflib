/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
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
	name_(name)
{
}

void HttpAuth::addUser(const QString & name, const QByteArray & passwordHash)
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
	const Request::LoginPass loginPass = Request::getBasicAuth(auth);
	if (!loginPass.login.isEmpty()) {
		const QByteArray hash = users_.value(loginPass.login);
		if (!hash.isEmpty() && crypt::checkPassword(loginPass.password.toUtf8(), hash)) {
			checkedUsers_ << auth;
			return;
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
