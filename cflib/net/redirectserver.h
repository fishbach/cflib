/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cflib/net/requesthandler.h>

#include <QtCore>

namespace cflib { namespace net {

class RedirectServer : public RequestHandler
{
public:
	void addRedirectIf     (const QRegularExpression & test, const QByteArray & destUrl);
	void addRedirectIfNot  (const QRegularExpression & test, const QByteArray & destUrl);
	void addDefaultRedirect(const QByteArray & destUrl);

	void addForwardIf      (const QRegularExpression & test, const QByteArray & ip, quint16 port);
	void addForwardIfNot   (const QRegularExpression & test, const QByteArray & ip, quint16 port);
	void addDefaultForward (const QByteArray & ip, quint16 port);

protected:
	virtual void handleRequest(const Request & request);

private:
	struct Entry
	{
		bool isRedirect;
		bool isDefault;
		bool invert;
		QRegularExpression test;
		QByteArray destUrl;
		QByteArray ip;
		quint16 port;

		Entry(bool invert, const QRegularExpression & test, const QByteArray & destUrl) :
			isRedirect(true), isDefault(false), invert(invert), test(test), destUrl(destUrl), port(0) {}
		Entry(bool invert, const QRegularExpression & test, const QByteArray & ip, quint16 port) :
			isRedirect(false), isDefault(false), invert(invert), test(test), ip(ip), port(port) {}
		Entry(const QByteArray & destUrl) :
			isRedirect(true), isDefault(true), invert(false), destUrl(destUrl), port(0) {}
		Entry(const QByteArray & ip, quint16 port) :
			isRedirect(false), isDefault(true), invert(false), ip(ip), port(port) {}
	};
	QList<Entry> entries_;
};

}}	// namespace
