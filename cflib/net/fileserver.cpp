/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "fileserver.h"

#include <cflib/crypt/util.h>
#include <cflib/net/request.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

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
        .replace(QRegularExpression("<!--.*?-->"), "")
        .replace(QRegularExpression("^\\s+|\\s+$"), "")
        .replace(QRegularExpression("\\s+"), " ");

    QFile f(file);
    f.open(QFile::WriteOnly | QFile::Truncate);
    f.write(content.toUtf8());
}

}

// ============================================================================

FileServer::FileServer(const QString & path, bool parseHtml, uint threadCount, bool enableIndex, bool noCache, bool removeSlash, bool useHostAsDir) :
    FileServer(path, QString(), parseHtml, threadCount, enableIndex, noCache, removeSlash, useHostAsDir)
{
}

FileServer::FileServer(const QString & path, const char * prefix, bool parseHtml, uint threadCount, bool enableIndex, bool noCache, bool removeSlash, bool useHostAsDir) :
    FileServer(path, QString(prefix), parseHtml, threadCount, enableIndex, noCache, removeSlash, useHostAsDir)
{
}

FileServer::FileServer(const QString & path, const QString & prefix, bool parseHtml, uint threadCount, bool enableIndex, bool noCache, bool removeSlash, bool useHostAsDir) :
    ThreadVerify("FileServer", Worker, threadCount),
    path_(path),
    prefix_(prefix),
    parseHtml_(parseHtml),
    enableIndex_(enableIndex),
    noCache_(noCache),
    removeSlash_(removeSlash),
    useHostAsDir_(useHostAsDir),
    eTag_(crypt::random(4).toHex()),
    pathRE_("^(/(?:(?:.well-known|[_\\-\\w][._\\-\\w]*)(?:/[_\\-\\w][._\\-\\w]*)*/?)?)(?:\\?.*)?$"),
    endingRE_("\\.(\\w+)$"),
    elementRE_("<!\\s*(\\$|inc |if |else|end|etag|importmap)(.*?)!>")
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

void FileServer::add404File(const QRegularExpression & re, const QString & dest)
{
    redirects404_ << qMakePair(re, dest);
}

void FileServer::handleRequest(const Request & request)
{
    if (!verifyThreadCall(&FileServer::handleRequest, request)) return;

    QString path = request.getUri();

    // Is it for us?
    if (!prefix_.isEmpty()) {
        if (!path.startsWith(prefix_)) return;

        // remove trailing slash
        if (removeSlash_ && path.endsWith('/')) { request.sendRedirect(path.left(path.length() - 1).toUtf8()); return; }

        path.remove(0, prefix_.length());
        if (path.isEmpty()) path += '/';
    } else {
        // remove trailing slash
        if (removeSlash_ && path.length() > 1 && path.endsWith('/')) { request.sendRedirect(path.left(path.length() - 1).toUtf8()); return; }
    }

    // check eTag
    if (!noCache_ && request.getHeader("if-none-match") == eTag_) {
        request.sendRaw(
            "HTTP/1.1 304 Not Modified\r\n"
            << request.defaultHeaders() <<
            "Cache-Control: no-cache\r\n"
            "ETag: " << eTag_ << "\r\n"
            "Content-Type: text/html; charset=utf-8\r\n",

            "<html>\r\n"
            "<head><title>304 - Not Modified</title></head>\r\n"
            "<body>\r\n"
            "<h1>304 - Not Modified</h1>\r\n"
            "</body>\r\n"
            "</html>\r\n",
            false);
        return;
    }

    // check path for valid chars
    QRegularExpressionMatch reMatch = pathRE_.match(path);
    if (!reMatch.hasMatch()) {
        logInfo("invalid path: %1", path);
        return;
    } else {
        path = reMatch.captured(1);
    }

    logFunctionTraceParam("FileServer::handleRequest(%1)", path);

    // auto generate partial files
    bool isPart = false;
    if (parseHtml_ && path.endsWith("/index_part.html")) {
        isPart = true;
        path.remove(path.length() - 16, 16);
        if (path.isEmpty()) path = '/';
    }

    QString fullPath = path_;
    if (useHostAsDir_) {
        fullPath += '/';
        fullPath += request.getHeader("host");
    }
    fullPath += path;
    QFileInfo fi(fullPath);
    if (fi.isDir()) {
        if (enableIndex_) {
            request.addHeaderLine("Cache-Control: no-cache");
            if (request.isHEAD()) request.sendText("");
            else                  request.sendText(createIndex(fi.canonicalFilePath(), path));
            return;
        }
        fi.setFile(fullPath + "/index.html");
    }

    // check for redirects
    if (!fi.isReadable()) {
        bool wasRedirect = false;
        const QString origPath = request.getUri();
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
        request.addHeaderLine("Cache-Control: no-cache");
        if (!noCache_) request.addHeaderLine("ETag: " << eTag_);
        if (request.isHEAD()) request.sendText("");
        else if (parseHtml_)  request.sendText(parseHtml(fullPath, isPart, path));
        else                  request.sendText(util::readFile(fullPath));
        return;
    }

    QByteArray replyData;
    if (!request.isHEAD()) {
        if (parseHtml_ && (
            fullPath.endsWith(".css") ||
            fullPath.endsWith(".js" ) ||
            fullPath.endsWith(".mjs")))
        {
            replyData = parseHtml(fullPath, false, path).toUtf8();
        } else {
            replyData = util::readFile(fullPath);
        }
    }

    // deliver static content
    bool cache = true;
    bool compression = false;
    QByteArray contentType = "application/octet-stream";
    const QRegularExpressionMatch match = endingRE_.match(path);
    if (match.hasMatch()) {
        const QString ending = match.captured(1);
             if (ending == "htm" ) { cache = false; compression = true;  contentType = "text/html; charset=utf-8"; }
        else if (ending == "txt" ) { cache = false; compression = true;  contentType = "text/plain"; }
        else if (ending == "ico" ) { cache = true;  compression = false; contentType = "image/x-icon"; }
        else if (ending == "gif" ) { cache = true;  compression = false; contentType = "image/gif"; }
        else if (ending == "png" ) { cache = true;  compression = false; contentType = "image/png"; }
        else if (ending == "jpg" ) { cache = true;  compression = false; contentType = "image/jpeg"; }
        else if (ending == "jpeg") { cache = true;  compression = false; contentType = "image/jpeg"; }
        else if (ending == "svg" ) { cache = true;  compression = true;  contentType = "image/svg+xml"; }
        else if (ending == "js"  ) { cache = true;  compression = true;  contentType = "text/javascript; charset=utf-8"; }
        else if (ending == "mjs" ) { cache = true;  compression = true;  contentType = "text/javascript; charset=utf-8"; }
        else if (ending == "css" ) { cache = true;  compression = true;  contentType = "text/css; charset=utf-8"; }
        else if (ending == "data") { cache = true;  compression = true;  contentType = "application/octet-stream"; }
        else if (ending == "pdf" ) { cache = false; compression = true;  contentType = "application/pdf"; }
        else if (ending == "log" ) { cache = false; compression = true;  contentType = "text/plain"; }
        else if (ending == "md"  ) { cache = false; compression = true;  contentType = "text/markdown; charset=utf-8"; }
    }
    if (!noCache_ && cache) {
        request.addHeaderLine("Cache-Control: max-age=31536000");
    } else {
        request.addHeaderLine("Cache-Control: no-cache");
        if (!noCache_) request.addHeaderLine("ETag: " << eTag_);
    }
    if (request.isHEAD()) request.sendReply("", contentType);
    else                  request.sendReply(replyData, contentType, compression);
}

QString FileServer::parseHtml(const QString & fullPath, bool isPart, const QString & path,
    const QStringList & params) const
{
    logFunctionTraceParam("FileServer::parseHtml(%1, %2, %3, (%4))", fullPath, isPart, path, params.join(','));

    QString retval;
    QString html = util::readTextfile(fullPath);
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
        } else if (cmd == "etag") {
            if (skip) continue;
            retval += eTag_;
        } else if (cmd == "importmap") {
            if (skip) continue;
            retval += "<script type=\"importmap\">{\"imports\":{";
            QDirIterator it(path_, {"*.mjs"}, QDir::NoFilter, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
            const int len = path_.length() + 1;
            const QString suffix = '?' + eTag_ + '"';
            bool isFirst = true;
            while (it.hasNext()) {
                const QString file = it.next().mid(len);
                if (isFirst) isFirst = false;
                else retval += ',';
                retval << "\"/" << file << "\":\"./" << file << suffix;
            }
            retval += "}}</script>";
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
            writeHTMLFile(dest + "/404.html",        parseHtml(filePath, false, path));
        } else if (fileName.endsWith(".css")) {
            QString out = parseHtml(filePath, false, path);
            out.replace(QRegularExpression("(@import url\\(\".*?)\\?" + eTag_), "\\1");
            QFile f(dest + '/' + fileName);
            f.open(QFile::WriteOnly | QFile::Truncate);
            f.write(out.toUtf8());
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

QString FileServer::createIndex(const QString & fullPath, const QString & path)
{
    QString html;
    html << "<!DOCTYPE html>\r\n<html><head></head>\r\n<body>\r\n";

    QString backJumpPath;
    QString pathStart;
    if (!removeSlash_) {
        if (path != "/") {
            int pos = path.lastIndexOf('/');
            backJumpPath = prefix_ + path.left(pos);
        }
        pathStart = prefix_ + path + (path.endsWith('/') ? "" : "/");

    } else {
        // remove slash mode - create relative paths
        const QString lastPartOfPrefix = prefix_.split("/").last();
        const QStringList pathSplitted = path.split("/");
        if (path != "/") { // subdir
            // for the backjump path we must go two levels back and one forth
            // example:
            // /devlog/data1       -> ../devlog        (go to root -> subdir "data1")
            // /devlog/data1/data2 -> ../data1/data2   (go to devlog -> subdir "data1")
            if (pathSplitted.size() >= 3) { // subdir level 2 or greater
                backJumpPath = "../" + pathSplitted[pathSplitted.size() - 2];
            } else { // first subdir level
                backJumpPath = "../" + lastPartOfPrefix;
            }
            pathStart = pathSplitted.last() + "/";
        } else {
            pathStart = lastPartOfPrefix + "/";
        }
    }

    if (!backJumpPath.isNull()) {
        html << "<a href=\"" << backJumpPath << "\">..</a><br>\r\n";
    }

    for (const QString & entry : QDir(fullPath).entryList(
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
            QDir::Name | QDir::DirsFirst | QDir::IgnoreCase)) {
        html << "<a href=\"" << pathStart << entry << "\">" << entry << "</a><br>\r\n";
    }

    html << "</body></html>";
    return html;
}

}}    // namespace
