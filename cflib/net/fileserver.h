/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/net/requesthandler.h>
#include <cflib/util/threadverify.h>

namespace cflib::net {

class FileServer : public RequestHandler, public util::ThreadVerify
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

    void exportTo(const String & dest) const;
    void add404File(const CFRegex & re, const String & dest);
    void setAccessControlAllowOrigin(const ByteArray & origin) { accessControlAllowOrigin_ = origin; }

protected:
    virtual void handleRequest(const Request & request);

private:
    String parseHtml(const String & fullPath, bool isPart, const String & path,
        const StringList & params = StringList()) const;
    void exportDir(const String & fullPath, const String & path, const String & dest) const;
    String createIndex(const String & fullPath, const String & path);

private:
    const String path_;
    const String prefix_;
    const bool parseHtml_;
    const bool enableIndex_;
    const bool noCache_;
    const bool removeSlash_;
    const bool useHostAsDir_;
    const ByteArray eTag_;
    typedef Pair<CFRegex, String> Redirect;
    List<Redirect> redirects404_;
    const CFRegex pathRE_;
    const CFRegex endingRE_;
    const CFRegex elementRE_;
    ByteArray accessControlAllowOrigin_;
};

} // namespace
