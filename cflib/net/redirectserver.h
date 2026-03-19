/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/net/requesthandler.h>

#include <functional>

namespace cflib { namespace net {

class RedirectServer : public RequestHandler
{
public:
    typedef CFPair<ByteArray /*ip*/, cfuint16 /*port*/> DestHost;
    typedef std::function<ByteArray (const Request &                                 )> DestUrlFunc;
    typedef std::function<ByteArray (const Request &, const CFRegex::MatchResult &)> DestUrlReFunc;
    typedef std::function<DestHost   (const Request &                                 )> DestHostFunc;
    typedef std::function<DestHost   (const Request &, const CFRegex::MatchResult &)> DestHostReFunc;

public:
    void addValid          (const CFRegex & test);

    void addRedirectIf     (const CFRegex & test, const char * destUrl);
    void addRedirectIf     (const CFRegex & test, const ByteArray & destUrl);
    void addRedirectIf     (const CFRegex & test, DestUrlReFunc destUrlReFunc);
    void addRedirectIfNot  (const CFRegex & test, const char * destUrl);
    void addRedirectIfNot  (const CFRegex & test, const ByteArray & destUrl);
    void addDefaultRedirect(const char * destUrl);
    void addDefaultRedirect(const ByteArray & destUrl);
    void addDefaultRedirect(DestUrlFunc destUrlFunc);

    void addForwardIf      (const CFRegex & test, const ByteArray & ip, cfuint16 port);
    void addForwardIf      (const CFRegex & test, DestHostReFunc destHostReFunc);
    void addForwardIfNot   (const CFRegex & test, const ByteArray & ip, cfuint16 port);
    void addDefaultForward (const ByteArray & ip, cfuint16 port);
    void addDefaultForward (DestHostFunc destHostFunc);

protected:
    virtual void handleRequest(const Request & request);

private:
    struct Entry
    {
        bool isValid;
        bool isRedirect;
        bool isDefault;
        bool invert;
        CFRegex test;
        ByteArray destUrl;
        DestUrlFunc destUrlFunc;
        DestUrlReFunc destUrlReFunc;
        DestHost destHost;
        DestHostFunc destHostFunc;
        DestHostReFunc destHostReFunc;

        Entry(const CFRegex & test) :
            isValid(true), isRedirect(false), isDefault(false), invert(false), test(test) {}

        Entry(bool invert, const CFRegex & test, const ByteArray & destUrl) :
            isValid(false), isRedirect(true), isDefault(false), invert(invert), test(test), destUrl(destUrl) {}
        Entry(bool invert, const CFRegex & test, DestUrlReFunc destUrlReFunc) :
            isValid(false), isRedirect(true), isDefault(false), invert(invert), test(test), destUrlReFunc(destUrlReFunc) {}
        Entry(const ByteArray & destUrl) :
            isValid(false), isRedirect(true), isDefault(true), invert(false), destUrl(destUrl) {}
        Entry(DestUrlFunc destUrlFunc) :
            isValid(false), isRedirect(true), isDefault(true), invert(false), destUrlFunc(destUrlFunc) {}

        Entry(bool invert, const CFRegex & test, const DestHost & destHost) :
            isValid(false), isRedirect(false), isDefault(false), invert(invert), test(test), destHost(destHost) {}
        Entry(bool invert, const CFRegex & test, DestHostReFunc destHostReFunc) :
            isValid(false), isRedirect(false), isDefault(false), invert(invert), test(test), destHostReFunc(destHostReFunc) {}
        Entry(const DestHost & destHost) :
            isValid(false), isRedirect(false), isDefault(true), invert(false), destHost(destHost) {}
        Entry(DestHostFunc destHostFunc) :
            isValid(false), isRedirect(false), isDefault(true), invert(false), destHostFunc(destHostFunc) {}
    };
    CFList<Entry> entries_;
};

}}    // namespace
