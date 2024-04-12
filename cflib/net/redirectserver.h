/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/requesthandler.h>

#include <QtCore>
#include <functional>

namespace cflib { namespace net {

class RedirectServer : public RequestHandler
{
public:
    typedef QPair<QByteArray /*ip*/, quint16 /*port*/> DestHost;
    typedef std::function<QByteArray (const Request &                                 )> DestUrlFunc;
    typedef std::function<QByteArray (const Request &, const QRegularExpressionMatch &)> DestUrlReFunc;
    typedef std::function<DestHost   (const Request &                                 )> DestHostFunc;
    typedef std::function<DestHost   (const Request &, const QRegularExpressionMatch &)> DestHostReFunc;

public:
    void addValid          (const QRegularExpression & test);

    void addRedirectIf     (const QRegularExpression & test, const char * destUrl);
    void addRedirectIf     (const QRegularExpression & test, const QByteArray & destUrl);
    void addRedirectIf     (const QRegularExpression & test, DestUrlReFunc destUrlReFunc);
    void addRedirectIfNot  (const QRegularExpression & test, const char * destUrl);
    void addRedirectIfNot  (const QRegularExpression & test, const QByteArray & destUrl);
    void addDefaultRedirect(const char * destUrl);
    void addDefaultRedirect(const QByteArray & destUrl);
    void addDefaultRedirect(DestUrlFunc destUrlFunc);

    void addForwardIf      (const QRegularExpression & test, const QByteArray & ip, quint16 port);
    void addForwardIf      (const QRegularExpression & test, DestHostReFunc destHostReFunc);
    void addForwardIfNot   (const QRegularExpression & test, const QByteArray & ip, quint16 port);
    void addDefaultForward (const QByteArray & ip, quint16 port);
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
        QRegularExpression test;
        QByteArray destUrl;
        DestUrlFunc destUrlFunc;
        DestUrlReFunc destUrlReFunc;
        DestHost destHost;
        DestHostFunc destHostFunc;
        DestHostReFunc destHostReFunc;

        Entry(const QRegularExpression & test) :
            isValid(true), isRedirect(false), isDefault(false), invert(false), test(test) {}

        Entry(bool invert, const QRegularExpression & test, const QByteArray & destUrl) :
            isValid(false), isRedirect(true), isDefault(false), invert(invert), test(test), destUrl(destUrl) {}
        Entry(bool invert, const QRegularExpression & test, DestUrlReFunc destUrlReFunc) :
            isValid(false), isRedirect(true), isDefault(false), invert(invert), test(test), destUrlReFunc(destUrlReFunc) {}
        Entry(const QByteArray & destUrl) :
            isValid(false), isRedirect(true), isDefault(true), invert(false), destUrl(destUrl) {}
        Entry(DestUrlFunc destUrlFunc) :
            isValid(false), isRedirect(true), isDefault(true), invert(false), destUrlFunc(destUrlFunc) {}

        Entry(bool invert, const QRegularExpression & test, const DestHost & destHost) :
            isValid(false), isRedirect(false), isDefault(false), invert(invert), test(test), destHost(destHost) {}
        Entry(bool invert, const QRegularExpression & test, DestHostReFunc destHostReFunc) :
            isValid(false), isRedirect(false), isDefault(false), invert(invert), test(test), destHostReFunc(destHostReFunc) {}
        Entry(const DestHost & destHost) :
            isValid(false), isRedirect(false), isDefault(true), invert(false), destHost(destHost) {}
        Entry(DestHostFunc destHostFunc) :
            isValid(false), isRedirect(false), isDefault(true), invert(false), destHostFunc(destHostFunc) {}
    };
    QList<Entry> entries_;
};

}}    // namespace
