/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfregex.h>
#include <cflib/net/requesthandler.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class FileServer : public RequestHandler, public util::ThreadVerify
{
public:
    FileServer(const CFString & path,
        bool parseHtml = false, uint threadCount = 1,
        bool enableIndex = false, bool noCache = false,
        bool removeSlash = true, bool useHostAsDir = false);

    FileServer(const CFString & path, const char * prefix,
        bool parseHtml = false, uint threadCount = 1,
        bool enableIndex = false, bool noCache = false,
        bool removeSlash = true, bool useHostAsDir = false);

    FileServer(const CFString & path, const CFString & prefix,
        bool parseHtml = false, uint threadCount = 1,
        bool enableIndex = false, bool noCache = false,
        bool removeSlash = true, bool useHostAsDir = false);

    ~FileServer();

    void exportTo(const CFString & dest) const;
    void add404File(const CFRegex & re, const CFString & dest);
    void setAccessControlAllowOrigin(const CFByteArray & origin) { accessControlAllowOrigin_ = origin; }

protected:
    virtual void handleRequest(const Request & request);

private:
    CFString parseHtml(const CFString & fullPath, bool isPart, const CFString & path,
        const CFStringList & params = CFStringList()) const;
    void exportDir(const CFString & fullPath, const CFString & path, const CFString & dest) const;
    CFString createIndex(const CFString & fullPath, const CFString & path);

private:
    const CFString path_;
    const CFString prefix_;
    const bool parseHtml_;
    const bool enableIndex_;
    const bool noCache_;
    const bool removeSlash_;
    const bool useHostAsDir_;
    const CFByteArray eTag_;
    typedef CFPair<CFRegex, CFString> Redirect;
    CFList<Redirect> redirects404_;
    const CFRegex pathRE_;
    const CFRegex endingRE_;
    const CFRegex elementRE_;
    CFByteArray accessControlAllowOrigin_;
};

}}    // namespace
