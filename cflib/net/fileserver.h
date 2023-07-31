/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
	void add404File(const QRegularExpression & re, const QString & dest);

protected:
	virtual void handleRequest(const Request & request);

private:
	QString parseHtml(const QString & fullPath, bool isPart, const QString & path,
		const QStringList & params = QStringList()) const;
	void exportDir(const QString & fullPath, const QString & path, const QString & dest) const;

private:
	const QString path_;
	const bool parseHtml_;
	const QByteArray eTag_;
	typedef QPair<QRegularExpression, QString> Redirect;
	QList<Redirect> redirects404_;
	const QRegularExpression pathRE_;
	const QRegularExpression endingRE_;
	const QRegularExpression elementRE_;
};

}}	// namespace
