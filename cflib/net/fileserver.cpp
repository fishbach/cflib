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

#include "fileserver.h"

#include <cflib/http/request.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace http {

namespace {

QStringList splitParams(const QString & param)
{
	QStringList retval;

	QString str;
	bool isStr = false;
	bool isEsc = false;
	int i = 0;
	const QString p = param.simplified();
	while (i < p.length()) {
		const QChar c = p[i++];
		if (isEsc) {
			isEsc = false;
			if      (c == '\\') str += '\\';
			else if (c == '"')  str += '"';
		} else if (c == '\\') {
			isEsc = true;
		} else if (isStr) {
			if (c == '"') {
				isStr = false;
				retval << str;
				str.clear();
			} else str += c;
		} else {
			if (c == '"') isStr = true;
			else if (c == ' ') {
				if (!str.isEmpty()) {
					retval << str;
					str.clear();
				}
			} else {
				str += c;
			}
		}
	}
	if (!str.isEmpty()) retval << str;

	return retval;
}

inline QString handleVars(const QString & expr, const QString & path, const QStringList & params)
{
	QString retval = expr.trimmed();
	if (retval == "$path") retval = path;
	else if (retval[0] == '$') {
		bool ok;
		uint nr = retval.mid(1).toUInt(&ok) - 1;
		if (ok && nr < (uint)params.length()) retval = params[nr];
		else retval.clear();
	}
	return retval;
}

inline void handleVars(QStringList & vars, const QString & path, const QStringList & params)
{
	QMutableStringListIterator it(vars);
	while (it.hasNext()) {
		QString & var = it.next();
		var = handleVars(var, path, params);
	}
}

void writeHTMLFile(const QString & file, QString content)
{
	content
		.replace(QRegularExpression("^\\s+|\\s+$"), "")
		.replace(QRegularExpression("\\s+"), " ");

	QFile f(file);
	f.open(QFile::WriteOnly | QFile::Truncate);
	f.write(content.toUtf8());
}

}

// ============================================================================

FileServer::FileServer(const QString & path, bool parseHtml, uint threadCount) :
	ThreadVerify("FileServer", Worker, threadCount),
	path_(path),
	parseHtml_(parseHtml),
	pathRE_("^/([_\\-\\w][._\\-\\w]*(/[_\\-\\w][._\\-\\w]*)*/?)?$"),
	endingRE_("\\.(\\w+)$"),
	elementRE_("<!\\s*(\\$|inc |if |else|end)(.*?)!>")
{
}

FileServer::~FileServer()
{
	stopVerifyThread();
}

void FileServer::exportTo(const QString & dest) const
{
	exportDir(path_, "/", dest);
}

void FileServer::set404Redirect(const QRegularExpression & re, const QString & dest)
{
	redirects404_ << qMakePair(re, dest);
}

void FileServer::handleRequest(const Request & request)
{
	if (!verifyThreadCall(&FileServer::handleRequest, request)) return;

	// check path for valid chars
	QString path = request.getUrl().path();
	if (!pathRE_.match(path).hasMatch()) {
		logInfo("invalid path: %1", path);
		return;
	}

	logFunctionTraceParam("FileServer::handleRequest(%1)", path);

	// remove trailing slash
	if (path.length() > 1 && path.endsWith('/')) { request.sendRedirect(path.left(path.length() - 1).toLatin1()); return; }

	// auto generate partial files
	bool isPart = false;
	if (parseHtml_ && path.endsWith("/index_part.html")) {
		isPart = true;
		path.remove(path.length() - 16, 16);
		if (path.isEmpty()) path = '/';
	}

	QString fullPath = path_ + path;
	QFileInfo fi(fullPath);
	if (fi.isDir()) fi.setFile(fullPath + "/index.html");

	// check for redirects
	if (!fi.isReadable()) {
		bool wasRedirect = false;
		const QString origPath = request.getUrl().path();
		foreach (const Redirect & rd, redirects404_) {
			if (rd.first.match(origPath).hasMatch()) {
				isPart = false;
				fi.setFile(path_ + rd.second);
				if (fi.isReadable()) wasRedirect = true;
				break;
			}
		}
		if (!wasRedirect) {
			logInfo("file not found: %1", fullPath);
			return;
		}
	}

	fullPath = fi.canonicalFilePath();

	// parse html files
	if (fullPath.endsWith(".html")) {
		if (parseHtml_) request.sendText(parseHtml(fullPath, isPart, path));
		else            request.sendText(cflib::util::readFile(fullPath));
		return;
	}

	// deliver static content
	bool compression = false;
	QByteArray contentType = "application/octet-stream";
	const QRegularExpressionMatch match = endingRE_.match(path);
	if (match.hasMatch()) {
		const QString ending = match.captured(1);
		     if (ending == "htm" )  { compression = true;  contentType = "text/html; charset=utf-8"; }
		else if (ending == "ico" )  { compression = false; contentType = "image/x-icon"; }
		else if (ending == "gif" )  { compression = false; contentType = "image/gif"; }
		else if (ending == "png" )  { compression = false; contentType = "image/png"; }
		else if (ending == "jpg" )  { compression = false; contentType = "image/jpeg"; }
		else if (ending == "jpeg")  { compression = false; contentType = "image/jpeg"; }
		else if (ending == "js"  )  { compression = true;  contentType = "application/javascript; charset=utf-8"; }
		else if (ending == "css" )  { compression = true;  contentType = "text/css; charset=utf-8"; }
		else if (ending == "data" ) { compression = true;  contentType = "application/octet-stream"; }
	}
	request.sendReply(cflib::util::readFile(fullPath), contentType, compression);
}

QString FileServer::parseHtml(const QString & fullPath, bool isPart, const QString & path,
	const QStringList & params) const
{
	logFunctionTraceParam("FileServer::parseHtml(%1, %2, %3, (%4))", fullPath, isPart, path, params.join(','));

	QString retval;
	QString html = cflib::util::readTextfile(fullPath);
	QStack<bool> ifStack;
	QRegularExpressionMatch m;
	while ((m = elementRE_.match(html)).hasMatch()) {
		int pos = m.capturedStart();
		const bool skip = !ifStack.isEmpty() && !ifStack.top();
		if (!skip) retval += html.left(pos);
		pos += m.capturedLength();
		html.remove(0, pos);

		const QString cmd   = m.captured(1);
		const QString param = m.captured(2);

		if (cmd == "inc ") {
			if (skip) continue;
			QStringList incParams = splitParams(param);
			handleVars(incParams, path, params);
			if (incParams.isEmpty()) continue;
			QString inc = incParams.takeFirst();
			if (inc == "nopart") {
				if (isPart) continue;
				if (incParams.isEmpty()) continue;
				inc = incParams.takeFirst();
			}
			if (inc.indexOf('/') == 0) inc = path_ + inc;
			else                       inc = QFileInfo(fullPath).canonicalPath() + '/' + inc;
			retval += parseHtml(inc, isPart, path, incParams);
		} else if (cmd == "$") {
			if (skip) continue;
			retval += handleVars('$' + param.trimmed(), path, params);
		} else if (cmd == "if ") {
			QStringList cond = splitParams(param);
			if (cond.size() != 3) {
				ifStack.push(false);
				continue;
			}
			handleVars(cond, path, params);

			const QString & lhs = cond[0];
			const QString & cmp = cond[1];
			const QString & rhs = cond[2];

			bool eval = false;
			if (cmp == "==") {
				eval = lhs == rhs;
			} else if (cmp == "!=") {
				eval = lhs != rhs;
			} else if (cmp == "startsWith") {
				eval = lhs.startsWith(rhs);
			} else if (cmp == "!startsWith") {
				eval = !lhs.startsWith(rhs);
			} else if (cmp == "endsWith") {
				eval = lhs.endsWith(rhs);
			} else if (cmp == "!endsWith") {
				eval = !lhs.endsWith(rhs);
			} else if (cmp == "contains") {
				eval = lhs.indexOf(rhs) != -1;
			} else if (cmp == "!contains") {
				eval = lhs.indexOf(rhs) == -1;
			}

			ifStack.push(eval);
		} else if (cmd == "else") {
			ifStack.top() = skip;
		} else if (cmd == "end") {
			ifStack.pop();
		}
	}

	retval += html;
	return retval;
}

void FileServer::exportDir(const QString & fullPath, const QString & path, const QString & dest) const
{
	if (path == "/include") return;

	foreach (const QFileInfo & info, QDir(fullPath).entryInfoList(QDir::Files)) {
		const QString fileName = info.fileName();
		const QString filePath = info.canonicalFilePath();
		QDir().mkpath(dest);
		if (fileName == "index.html") {
			writeHTMLFile(dest + "/index.html",      parseHtml(filePath, false, path));
			writeHTMLFile(dest + "/index_part.html", parseHtml(filePath, true,  path));
		} else if (fileName == "404.html") {
			writeHTMLFile(dest + "/404.html", parseHtml(filePath, false, path));
		} else {
			const QString destFile = dest + '/' + fileName;
			QFile::remove(destFile);
			QFile::copy(filePath, destFile);
		}
	}

	QString p = path;
	if (path.length() > 1) p += '/';
	foreach (const QFileInfo & info, QDir(fullPath).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		exportDir(info.canonicalFilePath(), p + info.fileName(), dest + '/' + info.fileName());
	}
}

}}	// namespace
