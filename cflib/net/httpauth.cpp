/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "httpauth.h"

#include <cflib/crypt/util.h>
#include <cflib/net/request.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

HttpAuth::HttpAuth(const QByteArray & name, const QString & htpasswd) :
    name_(name), htpasswd_(htpasswd)
{
}

void HttpAuth::addUser(const QString & name, const QByteArray & passwordHash)
{
    users_[name] = passwordHash;
}

void HttpAuth::reset()
{
    users_.clear();
    checkedUsers_.clear();
}

void HttpAuth::handleRequest(const Request & request)
{
    if (!htpasswd_.isEmpty()) {
        QFileInfo fi(htpasswd_);
        if (!fi.isReadable()) {
            htpasswdLastMod_ = QDateTime();
            users_.clear();
            checkedUsers_.clear();
            logWarn("Cannot read HTTP Basic Auth file %1", htpasswd_);
        } else if (fi.lastModified() != htpasswdLastMod_) {
            htpasswdLastMod_ = fi.lastModified();
            users_.clear();
            checkedUsers_.clear();

            QFile f(htpasswd_);
            f.open(QFile::ReadOnly);
            while (!f.atEnd()) {
                QStringList parts = QString::fromUtf8(f.readLine()).split(':');
                if (parts.size() == 2) {
                    users_[parts[0].trimmed()] = parts[1].trimmed().toUtf8();
                }
            }
            logInfo("loaded HTTP Basic Auth file %1 with %2 entries", htpasswd_, users_.size());
        }
    }

    const QByteArray auth = request.getHeader("authorization");
    if (checkedUsers_.contains(auth)) return;

    const Request::LoginPass loginPass = Request::getBasicAuth(auth);
    if (!loginPass.login.isEmpty()) {
        const QByteArray hash = users_.value(loginPass.login);
        if (!hash.isEmpty() && crypt::checkPassword(loginPass.password.toUtf8(), hash)) {
            checkedUsers_ << auth;
            return;
        }
    }

    request.sendRaw(
        "HTTP/1.1 401 Not Authorized\r\n"
        "WWW-Authenticate: Basic realm=\"" << name_ << "\"\r\n"
        << request.defaultHeaders() <<
        "Content-Type: text/html; charset=utf-8\r\n",

        "<html>\r\n"
        "<head><title>401 - Not Authorized</title></head>\r\n"
        "<body>\r\n"
        "<h1>401 - Not Authorized</h1>\r\n"
        "</body>\r\n"
        "</html>\r\n",
        false);
}

}}    // namespace
