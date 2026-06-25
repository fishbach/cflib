/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/fileserver/fileserverbase.h>
#include <cflib/net/requesthandler.h>
#include <cflib/util/threadverify.h>

namespace cflib::net::fileserver {

class FileServer : public FileServerBase, public RequestHandler, public util::ThreadVerify
{
public:
    FileServer(const String & path,
        bool parseHtml = false, uint threadCount = 1,
        bool enableIndex = false, bool noCache = false,
        bool removeSlash = true, bool useHostAsDir = false);

    FileServer(const String & path, const char * prefix,
        bool parseHtml = false, uint threadCount = 1,
        bool enableIndex = false, bool noCache = false,
        bool removeSlash = true, bool useHostAsDir = false);

    FileServer(const String & path, const String & prefix,
        bool parseHtml = false, uint threadCount = 1,
        bool enableIndex = false, bool noCache = false,
        bool removeSlash = true, bool useHostAsDir = false);

    ~FileServer();

    void add404File(const Regex & re, const String & dest);
    void setAccessControlAllowOrigin(const ByteArray & origin) { accessControlAllowOrigin_ = origin; }

protected:
    void handleRequest(const Request & request) override;

private:
    String createIndex(const String & fullPath, const String & path);

private:
    const bool parseHtml_;
    const bool enableIndex_;
    const bool noCache_;
    const bool useHostAsDir_;
    const Regex pathRE_;
    const Regex endingRE_;
    typedef Pair<Regex, String> Redirect;
    List<Redirect> redirects404_;
    ByteArray accessControlAllowOrigin_;
};

} // namespace
