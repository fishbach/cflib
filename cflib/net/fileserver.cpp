/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "fileserver.h"

#include <cflib/base.h>
#include <cflib/crypt/util.h>
#include <cflib/net/request.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <dirent.h>
#include <stack>
#include <sys/stat.h>
#include <unistd.h>

USE_LOG(LogCat::Http)

namespace cflib::net {

namespace {

StringList splitParams(const String & param)
{
    StringList retval;

    String str;
    bool isStr = false;
    bool isEsc = false;
    int i = 0;
    const String p = param.simplified();
    while (i < (int)p.length()) {
        const char c = p[i++];
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

inline String handleVars(const String & expr, const String & path, const StringList & params)
{
    String retval = expr.trimmed();
    if (retval == "$path") retval = path;
    else if (retval[0] == '$') {
        bool ok;
        uint nr = retval.mid(1).toUInt(&ok) - 1;
        if (ok && nr < (uint)params.size()) retval = params[nr];
        else retval.clear();
    }
    return retval;
}

inline void handleVars(StringList & vars, const String & path, const StringList & params)
{
    for (auto & var : vars) {
        var = handleVars(var, path, params);
    }
}

void writeHTMLFile(const String & file, String content)
{
    static const Regex commentRe("<!--.*?-->");
    static const Regex trimRe("^\\s+|\\s+$");
    static const Regex spaceRe("\\s+");
    content = commentRe.replaceAll(content, "");
    content = trimRe.replaceAll(content, " ");
    content = spaceRe.replaceAll(content, " ");

    cflib::util::writeFile(file, content.toUtf8());
}

// Check if path is a directory
inline bool isDirectory(const String & path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

// Check if a file is readable
inline bool isReadable(const String & path) {
    return access(path.c_str(), R_OK) == 0;
}

// Get canonical path
inline String canonicalPath(const String & path) {
    char * real = realpath(path.c_str(), nullptr);
    if (!real) return path;
    String result(real);
    free(real);
    return result;
}

// Get directory part of path
inline String dirName(const String & path) {
    ssize_t pos = path.lastIndexOf("/");
    if (pos < 0) return ".";
    return path.left(pos);
}

}

// ============================================================================

FileServer::FileServer(const String & path, bool parseHtml, uint threadCount, bool enableIndex, bool noCache, bool removeSlash, bool useHostAsDir) :
    FileServer(path, String(), parseHtml, threadCount, enableIndex, noCache, removeSlash, useHostAsDir)
{
}

FileServer::FileServer(const String & path, const char * prefix, bool parseHtml, uint threadCount, bool enableIndex, bool noCache, bool removeSlash, bool useHostAsDir) :
    FileServer(path, String(prefix), parseHtml, threadCount, enableIndex, noCache, removeSlash, useHostAsDir)
{
}

FileServer::FileServer(const String & path, const String & prefix, bool parseHtml, uint threadCount, bool enableIndex, bool noCache, bool removeSlash, bool useHostAsDir) :
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

void FileServer::exportTo(const String & dest) const
{
    exportDir(path_, "/", dest);
}

void FileServer::add404File(const Regex & re, const String & dest)
{
    redirects404_ << Pair<Regex, String>(re, dest);
}

void FileServer::handleRequest(const Request & request)
{
    if (!verifyThreadCall(&FileServer::handleRequest, request)) return;

    String path = request.getUri();

    // Is it for us?
    if (!prefix_.isEmpty()) {
        if (!path.startsWith(prefix_)) return;

        // remove trailing slash
        if (removeSlash_ && path.endsWith("/")) { request.sendRedirect(path.left(path.length() - 1).toUtf8()); return; }

        path.remove(0, prefix_.length());
        if (path.isEmpty()) path += '/';
    } else {
        // remove trailing slash
        if (removeSlash_ && path.length() > 1 && path.endsWith("/")) { request.sendRedirect(path.left(path.length() - 1).toUtf8()); return; }
    }

    if (!accessControlAllowOrigin_.isNull()) {
        ByteArray acoLine = "Access-Control-Allow-Origin: ";
        acoLine << accessControlAllowOrigin_;
        request.addHeaderLine(acoLine);
    }

    // check eTag
    if (!noCache_ && request.getHeader("if-none-match") == eTag_) {
        ByteArray hdr = "HTTP/1.1 304 Not Modified\r\n";
        hdr << request.defaultHeaders()
            << "Cache-Control: no-cache\r\n"
               "ETag: " << eTag_ << "\r\n"
               "Content-Type: text/html; charset=utf-8\r\n";
        request.sendRaw(hdr,
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
    Regex::MatchResult reMatch = pathRE_.matchResult(path);
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
        if (path.isEmpty()) path = "/";
    }

    String fullPath = path_;
    if (useHostAsDir_) {
        fullPath += '/';
        fullPath += request.getHeader("host");
    }
    fullPath += path;

    bool fileIsDir = isDirectory(fullPath);
    if (fileIsDir) {
        if (enableIndex_) {
            request.addHeaderLine("Cache-Control: no-cache");
            if (request.isHEAD()) request.sendText("");
            else                  request.sendText(createIndex(canonicalPath(fullPath), path));
            return;
        }
        fullPath += "/index.html";
    }

    bool fileReadable = isReadable(fullPath);

    // check for redirects
    if (!fileReadable) {
        bool wasRedirect = false;
        const String origPath = request.getUri();
        for (const auto & rd : redirects404_) {
            if (rd.first.match(origPath)) {
                isPart = false;
                fullPath = path_ + rd.second;
                fileReadable = isReadable(fullPath);
                if (fileReadable) wasRedirect = true;
                break;
            }
        }
        if (!wasRedirect) {
            logInfo("file not found: %1", fullPath);
            return;
        }
    }

    fullPath = canonicalPath(fullPath);

    // parse html files
    if (fullPath.endsWith(".html")) {
        request.addHeaderLine("Cache-Control: no-cache");
        if (!noCache_) { ByteArray el = "ETag: "; el << eTag_; request.addHeaderLine(el); }
        if (request.isHEAD()) request.sendText("");
        else if (parseHtml_)  request.sendText(parseHtml(fullPath, isPart, path));
        else                  request.sendText(String(util::readFile(fullPath)));
        return;
    }

    ByteArray replyData;
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
    ByteArray contentType = "application/octet-stream";
    const Regex::MatchResult match = endingRE_.matchResult(path);
    if (match.hasMatch()) {
        const String ending = match.captured(1);
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
        else if (ending == "wasm") { cache = false; compression = true;  contentType = "application/wasm"; }
    }
    if (!noCache_ && cache) {
        request.addHeaderLine("Cache-Control: max-age=31536000");
    } else {
        request.addHeaderLine("Cache-Control: no-cache");
        if (!noCache_) { ByteArray el = "ETag: "; el << eTag_; request.addHeaderLine(el); }
    }
    if (request.isHEAD()) request.sendReply("", contentType);
    else                  request.sendReply(replyData, contentType, compression);
}

String FileServer::parseHtml(const String & fullPath, bool isPart, const String & path,
    const StringList & params) const
{
    logFunctionTraceParam("FileServer::parseHtml(%1, %2, %3, (%4))", fullPath, isPart, path, params.join(','));

    String retval;
    String html = util::readTextfile(fullPath);
    std::stack<bool> ifStack;
    Regex::MatchResult m;
    while ((m = elementRE_.matchResult(html)).hasMatch()) {
        int pos = m.capturedStart();
        const bool skip = !ifStack.empty() && !ifStack.top();
        if (!skip) retval += html.left(pos);
        pos += m.capturedLength();
        html.remove(0, pos);

        const String cmd   = m.captured(1);
        const String param = m.captured(2);

        if (cmd == "inc ") {
            if (skip) continue;
            StringList incParams = splitParams(param);
            handleVars(incParams, path, params);
            if (incParams.empty()) continue;
            String inc = incParams.takeFirst();
            if (inc == "nopart") {
                if (isPart) continue;
                if (incParams.empty()) continue;
                inc = incParams.takeFirst();
            }
            if (inc.indexOf('/') == 0) inc = path_ + inc;
            else                       inc = dirName(canonicalPath(fullPath)) + "/" + inc;
            retval += parseHtml(inc, isPart, path, incParams);
        } else if (cmd == "$") {
            if (skip) continue;
            retval += handleVars(String("$") + param.trimmed(), path, params);
        } else if (cmd == "etag") {
            if (skip) continue;
            retval += eTag_;
        } else if (cmd == "importmap") {
            if (skip) continue;
            retval += "<script type=\"importmap\">{\"imports\":{";
            // Walk directory tree for .mjs files
            std::function<void(const String &)> walkMjs;
            const int len = path_.length() + 1;
            const String suffix = String("?") + eTag_ + "\"";
            bool isFirst = true;
            walkMjs = [&](const String & dir) {
                DIR * d = opendir(dir.c_str());
                if (!d) return;
                struct dirent * ent;
                while ((ent = readdir(d)) != nullptr) {
                    String name(ent->d_name);
                    if (name == "." || name == "..") continue;
                    String full = dir + "/" + name;
                    struct stat st;
                    if (stat(full.c_str(), &st) != 0) continue;
                    if (S_ISDIR(st.st_mode)) {
                        walkMjs(full);
                    } else if (name.endsWith(".mjs")) {
                        String file = full.mid(len);
                        if (isFirst) isFirst = false;
                        else retval += ',';
                        retval += "\"/";
                        retval += file;
                        retval += "\":\"./";
                        retval += file;
                        retval += suffix;
                    }
                }
                closedir(d);
            };
            walkMjs(path_);
            retval += "}}</script>";
        } else if (cmd == "if ") {
            StringList cond = splitParams(param);
            if ((int)cond.size() != 3) {
                ifStack.push(false);
                continue;
            }
            handleVars(cond, path, params);

            const String & lhs = cond[0];
            const String & cmp = cond[1];
            const String & rhs = cond[2];

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

void FileServer::exportDir(const String & fullPath, const String & path, const String & dest) const
{
    if (path == "/include") return;

    DIR * d = opendir(fullPath.c_str());
    if (!d) return;

    // Create destination directory
    cflib::util::mkPath(dest);

    struct dirent * ent;
    while ((ent = readdir(d)) != nullptr) {
        String name(ent->d_name);
        if (name == "." || name == "..") continue;
        String filePath = canonicalPath(fullPath + "/" + name);
        struct stat st;
        if (stat(filePath.c_str(), &st) != 0) continue;

        if (S_ISREG(st.st_mode)) {
            if (name == "index.html") {
                writeHTMLFile(dest + "/index.html",      parseHtml(filePath, false, path));
                writeHTMLFile(dest + "/index_part.html", parseHtml(filePath, true,  path));
            } else if (name == "404.html") {
                writeHTMLFile(dest + "/404.html",        parseHtml(filePath, false, path));
            } else if (String(name).endsWith(".css")) {
                String out = parseHtml(filePath, false, path);
                { Regex importRe(String("(@import url\\(\".*?)\\?") + String(eTag_)); out = importRe.replace(out, "$1"); }
                cflib::util::writeFile(dest + "/" + name, out.toUtf8());
            } else {
                cflib::util::copyFile(filePath, dest + "/" + name);
            }
        }
    }
    closedir(d);

    // Process subdirectories
    d = opendir(fullPath.c_str());
    if (!d) return;
    String p = path;
    if (path.length() > 1) p += '/';
    while ((ent = readdir(d)) != nullptr) {
        String name(ent->d_name);
        if (name == "." || name == "..") continue;
        String subPath = fullPath + "/" + name;
        struct stat st;
        if (stat(subPath.c_str(), &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            exportDir(canonicalPath(subPath), p + name, dest + "/" + name);
        }
    }
    closedir(d);
}

String FileServer::createIndex(const String & fullPath, const String & path)
{
    String html;
    html << "<!DOCTYPE html>\r\n<html><head></head>\r\n<body>\r\n";

    String backJumpPath;
    String pathStart;
    if (!removeSlash_) {
        if (path != "/") {
            int pos = path.lastIndexOf("/");
            backJumpPath = prefix_ + path.left(pos);
        }
        pathStart = prefix_ + path + (path.endsWith("/") ? "" : "/");

    } else {
        // remove slash mode - create relative paths
        const String lastPartOfPrefix = prefix_.split("/").back();
        const StringList pathSplitted = path.split("/");
        if (path != "/") { // subdir
            if ((int)pathSplitted.size() >= 3) { // subdir level 2 or greater
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

    // List directory entries
    DIR * d = opendir(fullPath.c_str());
    if (d) {
        // Collect entries, then sort
        StringList dirs, files;
        struct dirent * ent;
        while ((ent = readdir(d)) != nullptr) {
            String name(ent->d_name);
            if (name == "." || name == "..") continue;
            struct stat st;
            String full = fullPath + "/" + name;
            if (stat(full.c_str(), &st) != 0) continue;
            if (S_ISDIR(st.st_mode)) dirs.push_back(name);
            else files.push_back(name);
        }
        closedir(d);

        dirs.sort();
        files.sort();
        for (const auto & entry : dirs) {
            html << "<a href=\"" << pathStart << entry << "\">" << entry << "</a><br>\r\n";
        }
        for (const auto & entry : files) {
            html << "<a href=\"" << pathStart << entry << "\">" << entry << "</a><br>\r\n";
        }
    }

    html << "</body></html>";
    return html;
}

} // namespace
