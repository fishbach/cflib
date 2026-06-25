/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "fileserver.h"

#include <cflib/base.h>
#include <cflib/net/request.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <dirent.h>
#include <stack>
#include <sys/stat.h>
#include <unistd.h>

USE_LOG(LogCat::Http)

namespace cflib::net::fileserver {

namespace {

// Check if path is a directory
inline bool isDirectory(const String & path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

// Check if a file is readable
inline bool isReadable(const String & path) {
    return access(path.c_str(), R_OK) == 0;
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
    FileServerBase(path, prefix, removeSlash),
    ThreadVerify("FileServer", Worker, threadCount),
    parseHtml_(parseHtml),
    enableIndex_(enableIndex),
    noCache_(noCache),
    useHostAsDir_(useHostAsDir),
    pathRE_("^(/(?:(?:.well-known|[_\\-\\w][._\\-\\w]*)(?:/[_\\-\\w][._\\-\\w]*)*/?)?)(?:\\?.*)?$"),
    endingRE_("\\.(\\w+)$")
{
}

FileServer::~FileServer()
{
    stopVerifyThread();
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
            else                  request.sendText(createIndex(util::canonicalPath(fullPath), path));
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

    fullPath = util::canonicalPath(fullPath);

    // parse html files
    if (fullPath.endsWith(".html")) {
        request.addHeaderLine("Cache-Control: no-cache");
        if (!noCache_) { ByteArray el = "ETag: "; el << eTag_; request.addHeaderLine(el); }
        if (request.isHEAD()) request.sendText("");
        else if (parseHtml_)  request.sendText(parseHtml(fullPath, isPart, path));
        else                  request.sendText(String(File::read(fullPath)));
        return;
    }

    ByteArray replyData;
    if (!request.isHEAD()) {
        if (parseHtml_ && (
            fullPath.endsWith(".css") ||
            fullPath.endsWith(".js")))
        {
            replyData = parseHtml(fullPath, false, path).toUtf8();
        } else {
            replyData = File::read(fullPath);
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
