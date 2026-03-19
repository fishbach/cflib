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

namespace cflib::net {

class RedirectServer : public RequestHandler
{
public:
    typedef Pair<ByteArray /*ip*/, cfuint16 /*port*/> DestHost;
    typedef std::function<ByteArray (const Request &                                 )> DestUrlFunc;
    typedef std::function<ByteArray (const Request &, const Regex::MatchResult &)> DestUrlReFunc;
    typedef std::function<DestHost   (const Request &                                 )> DestHostFunc;
    typedef std::function<DestHost   (const Request &, const Regex::MatchResult &)> DestHostReFunc;

public:
    void addValid          (const Regex & test);

    void addRedirectIf     (const Regex & test, const char * destUrl);
    void addRedirectIf     (const Regex & test, const ByteArray & destUrl);
    void addRedirectIf     (const Regex & test, DestUrlReFunc destUrlReFunc);
    void addRedirectIfNot  (const Regex & test, const char * destUrl);
    void addRedirectIfNot  (const Regex & test, const ByteArray & destUrl);
    void addDefaultRedirect(const char * destUrl);
    void addDefaultRedirect(const ByteArray & destUrl);
    void addDefaultRedirect(DestUrlFunc destUrlFunc);

    void addForwardIf      (const Regex & test, const ByteArray & ip, cfuint16 port);
    void addForwardIf      (const Regex & test, DestHostReFunc destHostReFunc);
    void addForwardIfNot   (const Regex & test, const ByteArray & ip, cfuint16 port);
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
        Regex test;
        ByteArray destUrl;
        DestUrlFunc destUrlFunc;
        DestUrlReFunc destUrlReFunc;
        DestHost destHost;
        DestHostFunc destHostFunc;
        DestHostReFunc destHostReFunc;

        Entry(const Regex & test) :
            isValid(true), isRedirect(false), isDefault(false), invert(false), test(test) {}

        Entry(bool invert, const Regex & test, const ByteArray & destUrl) :
            isValid(false), isRedirect(true), isDefault(false), invert(invert), test(test), destUrl(destUrl) {}
        Entry(bool invert, const Regex & test, DestUrlReFunc destUrlReFunc) :
            isValid(false), isRedirect(true), isDefault(false), invert(invert), test(test), destUrlReFunc(destUrlReFunc) {}
        Entry(const ByteArray & destUrl) :
            isValid(false), isRedirect(true), isDefault(true), invert(false), destUrl(destUrl) {}
        Entry(DestUrlFunc destUrlFunc) :
            isValid(false), isRedirect(true), isDefault(true), invert(false), destUrlFunc(destUrlFunc) {}

        Entry(bool invert, const Regex & test, const DestHost & destHost) :
            isValid(false), isRedirect(false), isDefault(false), invert(invert), test(test), destHost(destHost) {}
        Entry(bool invert, const Regex & test, DestHostReFunc destHostReFunc) :
            isValid(false), isRedirect(false), isDefault(false), invert(invert), test(test), destHostReFunc(destHostReFunc) {}
        Entry(const DestHost & destHost) :
            isValid(false), isRedirect(false), isDefault(true), invert(false), destHost(destHost) {}
        Entry(DestHostFunc destHostFunc) :
            isValid(false), isRedirect(false), isDefault(true), invert(false), destHostFunc(destHostFunc) {}
    };
    List<Entry> entries_;
};

} // namespace
