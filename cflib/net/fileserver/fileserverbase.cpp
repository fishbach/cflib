/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "fileserverbase.h"

#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <dirent.h>
#include <stack>
#include <sys/stat.h>
#include <unistd.h>

USE_LOG(LogCat::Http)

namespace cflib::net::fileserver {

namespace {

StringList splitParams(const String & param)
{
    StringList retval;

    String str;
    bool isStr = false;
    bool isEsc = false;
    int i = 0;
    const String p = param.simplified();
    while (i < (int)p.length()) {
        const char c = p[i++];
        if (isEsc) {
            isEsc = false;
            if      (c == '\\') str += '\\';
            else if (c == '"')  str += '"';
        } else if (c == '\\') {
            isEsc = true;
        } else if (isStr) {
            if (c == '"') {
                isStr = false;
                retval << str;
                str.clear();
            } else str += c;
        } else {
            if (c == '"') isStr = true;
            else if (c == ' ') {
                if (!str.isEmpty()) {
                    retval << str;
                    str.clear();
                }
            } else {
                str += c;
            }
        }
    }
    if (!str.isEmpty()) retval << str;

    return retval;
}

inline String handleVars(const String & expr, const String & path, const StringList & params)
{
    String retval = expr.trimmed();
    if (retval == "$path") retval = path;
    else if (retval[0] == '$') {
        bool ok;
        uint nr = retval.mid(1).toUInt(&ok) - 1;
        if (ok && nr < (uint)params.size()) retval = params[nr];
        else retval.clear();
    }
    return retval;
}

inline void handleVars(StringList & vars, const String & path, const StringList & params)
{
    for (auto & var : vars) {
        var = handleVars(var, path, params);
    }
}

void writeHTMLFile(const String & file, String content)
{
    static const Regex commentRe("<!--.*?-->");
    static const Regex trimRe("^\\s+|\\s+$");
    static const Regex spaceRe("\\s+");
    content = commentRe.replaceAll(content, "");
    content = trimRe.replaceAll(content, " ");
    content = spaceRe.replaceAll(content, " ");

    File::write(file, content.toUtf8());
}

// Get directory part of path
inline String dirName(const String & path) {
    ssize_t pos = path.lastIndexOf("/");
    if (pos < 0) return ".";
    return path.left(pos);
}

}

// ============================================================================

FileServerBase::FileServerBase(const String & path, const String & prefix, bool removeSlash) :
    path_(path),
    prefix_(prefix),
    removeSlash_(removeSlash),
    eTag_(util::unsafeRandom(4).toHex()),
    elementRE_("<!\\s*(\\$|inc |if |else|end|etag|importmap)(.*?)!>")
{
}

void FileServerBase::exportTo(const String & dest) const
{
    exportDir(path_, "/", dest);
}

String FileServerBase::parseHtml(const String & fullPath, bool isPart, const String & path,
    const StringList & params) const
{
    logFunctionTraceParam("FileServerBase::parseHtml(%1, %2, %3, (%4))", fullPath, isPart, path, params.join(','));

    String retval;
    String html = File::readUtf8(fullPath);
    std::stack<bool> ifStack;
    Regex::MatchResult m;
    while ((m = elementRE_.matchResult(html)).hasMatch()) {
        int pos = m.capturedStart();
        const bool skip = !ifStack.empty() && !ifStack.top();
        if (!skip) retval += html.left(pos);
        pos += m.capturedLength();
        html.remove(0, pos);

        const String cmd   = m.captured(1);
        const String param = m.captured(2);

        if (cmd == "inc ") {
            if (skip) continue;
            StringList incParams = splitParams(param);
            handleVars(incParams, path, params);
            if (incParams.isEmpty()) continue;
            String inc = incParams.takeFirst();
            if (inc == "nopart") {
                if (isPart) continue;
                if (incParams.isEmpty()) continue;
                inc = incParams.takeFirst();
            }
            if (inc.indexOf('/') == 0) inc = path_ + inc;
            else                       inc = dirName(util::canonicalPath(fullPath)) + "/" + inc;
            retval += parseHtml(inc, isPart, path, incParams);
        } else if (cmd == "$") {
            if (skip) continue;
            retval += handleVars(String("$") + param.trimmed(), path, params);
        } else if (cmd == "etag") {
            if (skip) continue;
            retval += eTag_;
        } else if (cmd == "importmap") {
            if (skip) continue;
            retval += "<script type=\"importmap\">{\"imports\":{";
            // Walk directory tree for .js files
            std::function<void(const String &)> walkMjs;
            const int len = path_.length() + 1;
            const String suffix = String("?") + eTag_ + "\"";
            bool isFirst = true;
            walkMjs = [&](const String & dir) {
                DIR * d = opendir(dir.c_str());
                if (!d) return;
                struct dirent * ent;
                while ((ent = readdir(d)) != nullptr) {
                    String name(ent->d_name);
                    if (name == "." || name == "..") continue;
                    String full = dir + "/" + name;
                    struct stat st;
                    if (stat(full.c_str(), &st) != 0) continue;
                    if (S_ISDIR(st.st_mode)) {
                        walkMjs(full);
                    } else if (name.endsWith(".js")) {
                        String file = full.mid(len);
                        if (isFirst) isFirst = false;
                        else retval += ',';
                        retval += "\"/";
                        retval += file;
                        retval += "\":\"/";
                        retval += file;
                        retval += suffix;
                    }
                }
                closedir(d);
            };
            walkMjs(path_);
            retval += "}}</script>";
        } else if (cmd == "if ") {
            StringList cond = splitParams(param);
            if ((int)cond.size() != 3) {
                ifStack.push(false);
                continue;
            }
            handleVars(cond, path, params);

            const String & lhs = cond[0];
            const String & cmp = cond[1];
            const String & rhs = cond[2];

            bool eval = false;
            if (cmp == "==") {
                eval = lhs == rhs;
            } else if (cmp == "!=") {
                eval = lhs != rhs;
            } else if (cmp == "startsWith") {
                eval = lhs.startsWith(rhs);
            } else if (cmp == "!startsWith") {
                eval = !lhs.startsWith(rhs);
            } else if (cmp == "endsWith") {
                eval = lhs.endsWith(rhs);
            } else if (cmp == "!endsWith") {
                eval = !lhs.endsWith(rhs);
            } else if (cmp == "contains") {
                eval = lhs.indexOf(rhs) != -1;
            } else if (cmp == "!contains") {
                eval = lhs.indexOf(rhs) == -1;
            }

            ifStack.push(eval);
        } else if (cmd == "else") {
            ifStack.top() = skip;
        } else if (cmd == "end") {
            ifStack.pop();
        }
    }

    retval += html;
    return retval;
}

void FileServerBase::exportDir(const String & fullPath, const String & path, const String & dest) const
{
    if (path == "/include") return;

    DIR * d = opendir(fullPath.c_str());
    if (!d) return;

    // Create destination directory
    cflib::util::mkPath(dest);

    struct dirent * ent;
    while ((ent = readdir(d)) != nullptr) {
        String name(ent->d_name);
        if (name == "." || name == "..") continue;
        String filePath = util::canonicalPath(fullPath + "/" + name);
        struct stat st;
        if (stat(filePath.c_str(), &st) != 0) continue;

        if (S_ISREG(st.st_mode)) {
            if (name == "index.html") {
                writeHTMLFile(dest + "/index.html",      parseHtml(filePath, false, path));
                writeHTMLFile(dest + "/index_part.html", parseHtml(filePath, true,  path));
            } else if (name == "404.html") {
                writeHTMLFile(dest + "/404.html",        parseHtml(filePath, false, path));
            } else if (String(name).endsWith(".css")) {
                String out = parseHtml(filePath, false, path);
                { Regex importRe(String("(@import url\\(\".*?)\\?") + String(eTag_)); out = importRe.replace(out, "$1"); }
                File::write(dest + "/" + name, out.toUtf8());
            } else {
                cflib::util::copyFile(filePath, dest + "/" + name);
            }
        }
    }
    closedir(d);

    // Process subdirectories
    d = opendir(fullPath.c_str());
    if (!d) return;
    String p = path;
    if (path.length() > 1) p += '/';
    while ((ent = readdir(d)) != nullptr) {
        String name(ent->d_name);
        if (name == "." || name == "..") continue;
        String subPath = fullPath + "/" + name;
        struct stat st;
        if (stat(subPath.c_str(), &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            exportDir(util::canonicalPath(subPath), p + name, dest + "/" + name);
        }
    }
    closedir(d);
}

} // namespace
