/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfbytearray.h>
#include <cflib/base/cfstring.h>
#include <cflib/base/macros.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>

class CFFile
{
    CF_DISABLE_COPY(CFFile)
public:
    enum OpenMode {
        ReadOnly  = 0x01,
        WriteOnly = 0x02,
        ReadWrite = 0x03,
        Append    = 0x04,
        Truncate  = 0x08
    };

    enum Permission {
        ReadOwner  = 0x4000,
        WriteOwner = 0x2000,
        ReadUser   = 0x0400,
        WriteUser  = 0x0200,
        ReadGroup  = 0x0040,
        WriteGroup = 0x0020,
        ReadOther  = 0x0004,
        WriteOther = 0x0002
    };

    CFFile() : fp_(nullptr) {}
    explicit CFFile(const CFString & path) : path_(path.str()), fp_(nullptr) {}
    ~CFFile() { close(); }

    CFFile(CFFile && other) noexcept : path_(std::move(other.path_)), fp_(other.fp_) { other.fp_ = nullptr; }

    void setFileName(const CFString & path) { path_ = path.str(); }

    bool open(int fd, int mode) {
        const char * m = (mode & Append) ? "a" : ((mode & WriteOnly) ? "w" : "r");
        fp_ = fdopen(fd, m);
        return fp_ != nullptr;
    }

    bool open(int mode) {
        const char * m;
        if ((mode & Append))                              m = "ab";
        else if ((mode & Truncate) && (mode & WriteOnly)) m = "wb";
        else if (mode & WriteOnly)                        m = "wb";
        else                                              m = "rb";
        fp_ = fopen(path_.c_str(), m);
        return fp_ != nullptr;
    }

    void close() {
        if (fp_) { fclose(fp_); fp_ = nullptr; }
    }

    bool isOpen() const { return fp_ != nullptr; }

    cfint64 write(const CFByteArray & data) {
        if (!fp_) return -1;
        return (cfint64)fwrite(data.constData(), 1, data.size(), fp_);
    }
    cfint64 write(const char * data, cfsize_t len) {
        if (!fp_) return -1;
        return (cfint64)fwrite(data, 1, len, fp_);
    }

    void flush() { if (fp_) fflush(fp_); }

    CFByteArray readAll() {
        if (!fp_) return CFByteArray();
        CFByteArray result;
        char buf[4096];
        cfsize_t n;
        while ((n = fread(buf, 1, sizeof(buf), fp_)) > 0)
            result.append(buf, n);
        return result;
    }

    bool setPermissions(int perms) {
        if (path_.empty()) return false;
        mode_t m = 0;
        if (perms & ReadOwner)  m |= S_IRUSR;
        if (perms & WriteOwner) m |= S_IWUSR;
        if (perms & ReadUser)   m |= S_IRUSR;
        if (perms & WriteUser)  m |= S_IWUSR;
        if (perms & ReadGroup)  m |= S_IRGRP;
        if (perms & WriteGroup) m |= S_IWGRP;
        if (perms & ReadOther)  m |= S_IROTH;
        if (perms & WriteOther) m |= S_IWOTH;
        return chmod(path_.c_str(), m) == 0;
    }

    CFString fileName() const { return CFString(path_.c_str()); }
    CFString errorString() const { return CFString(strerror(errno)); }

    static bool exists(const CFString & path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }

private:
    std::string path_;
    FILE * fp_;
};
