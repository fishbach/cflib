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

#include <cflib/http/requesthandler.h>

#include <QtCore>

namespace cflib { namespace http {

class FileServer : public RequestHandler
{
public:
	FileServer(const QString & path);
	~FileServer();

	void exportTo(const QString & dest) const;
	void set404Redirect(const QRegExp & re, const QString & dest);

protected:
	virtual void handleRequest(const Request & request);

private:
	QString parseHtml(const QString & fullPath, bool isPart, const QString & path,
		const QStringList & params = QStringList()) const;
	void exportDir(const QString & fullPath, const QString & path, const QString & dest) const;

private:
	QString path_;
	typedef QPair<QRegExp, QString> Redirect;
	QList<Redirect> redirects404_;
};

}}	// namespace
