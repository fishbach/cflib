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
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class FileServer : public RequestHandler, public util::ThreadVerify
{
public:
	FileServer(const QString & path, bool parseHtml = true, uint threadCount = 1);
	~FileServer();

	void exportTo(const QString & dest) const;
	void set404Redirect(const QRegularExpression & re, const QString & dest);

protected:
	virtual void handleRequest(const Request & request);

private:
	QString parseHtml(const QString & fullPath, bool isPart, const QString & path,
		const QStringList & params = QStringList()) const;
	void exportDir(const QString & fullPath, const QString & path, const QString & dest) const;

private:
	const QString path_;
	const bool parseHtml_;
	typedef QPair<QRegularExpression, QString> Redirect;
	QList<Redirect> redirects404_;
	const QRegularExpression pathRE_;
	const QRegularExpression endingRE_;
	const QRegularExpression elementRE_;
};

}}	// namespace
