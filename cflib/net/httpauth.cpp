/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
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

#include <sys/stat.h>

USE_LOG(LogCat::Http)

namespace cflib::net {

HttpAuth::HttpAuth(const ByteArray & name, const String & htpasswd) :
    name_(name), htpasswd_(htpasswd)
{
}

void HttpAuth::addUser(const String & name, const ByteArray & passwordHash)
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
        struct stat st;
        bool readable = stat(htpasswd_.c_str(), &st) == 0 && S_ISREG(st.st_mode);
        if (!readable) {
            htpasswdLastMod_ = CFDateTime();
            users_.clear();
            checkedUsers_.clear();
            logWarn("Cannot read HTTP Basic Auth file %1", htpasswd_);
        } else {
            CFDateTime fileMod = CFDateTime::fromSecsSinceEpoch(st.st_mtime);
            if (fileMod.toSecsSinceEpoch() != htpasswdLastMod_.toSecsSinceEpoch()) {
                htpasswdLastMod_ = fileMod;
                users_.clear();
                checkedUsers_.clear();

                ByteArray content = util::readFile(htpasswd_);
                CFList<ByteArray> lines = content.split('\n');
                for (const auto & line : lines) {
                    CFList<ByteArray> parts = line.split(':');
                    if (parts.size() == 2) {
                        users_[String(parts[0].trimmed().toStdString())] = parts[1].trimmed();
                    }
                }
                logInfo("loaded HTTP Basic Auth file %1 with %2 entries", htpasswd_, users_.size());
            }
        }
    }

    const ByteArray auth = request.getHeader("authorization");
    if (cfContains(checkedUsers_, auth)) return;

    const Request::LoginPass loginPass = Request::getBasicAuth(auth);
    if (!loginPass.login.isEmpty()) {
        auto it = users_.find(loginPass.login);
        const ByteArray hash = it != users_.end() ? it->second : ByteArray();
        if (!hash.isEmpty() && crypt::checkPassword(loginPass.password.toUtf8(), hash)) {
            checkedUsers_.insert(auth);
            return;
        }
    }

    ByteArray hdr = "HTTP/1.1 401 Not Authorized\r\n"
        "WWW-Authenticate: Basic realm=\"";
    hdr << name_ << "\"\r\n" << request.defaultHeaders() << "Content-Type: text/html; charset=utf-8\r\n";
    request.sendRaw(hdr,
        "<html>\r\n"
        "<head><title>401 - Not Authorized</title></head>\r\n"
        "<body>\r\n"
        "<h1>401 - Not Authorized</h1>\r\n"
        "</body>\r\n"
        "</html>\r\n",
        false);
}

} // namespace
