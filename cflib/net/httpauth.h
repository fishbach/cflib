/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/requesthandler.h>

#include <QtCore>

namespace cflib { namespace net {

class HttpAuth : public RequestHandler
{
public:
	HttpAuth(const QByteArray & name);

	void addUser(const QByteArray & name, const QByteArray & passwordHash);
	void reset();

protected:
	virtual void handleRequest(const Request & request);

private:
	const QRegularExpression authRe_;
	const QByteArray name_;
	QMap<QByteArray, QByteArray> users_;
	QSet<QByteArray> checkedUsers_;
};

}}	// namespace
