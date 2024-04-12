/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/requesthandler.h>

#include <QtCore>

namespace cflib { namespace net {

class HttpAuth : public RequestHandler
{
public:
    HttpAuth(const QByteArray & name, const QString & htpasswd = QString());

    void addUser(const QString & name, const QByteArray & passwordHash);
    void reset();

protected:
    virtual void handleRequest(const Request & request);

private:
    const QByteArray name_;
    const QString htpasswd_;
    QDateTime htpasswdLastMod_;
    QMap<QString, QByteArray> users_;
    QSet<QByteArray> checkedUsers_;
};

}}    // namespace
