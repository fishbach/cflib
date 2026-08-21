/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/bytearray.h>
#include <cflib/base/string.h>
#include <cflib/base/macros.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>

namespace cflib::base {

class File
{
    CF_DISABLE_COPY(File)
public:
    enum OpenMode {
        ReadOnly  = 0x01,
        WriteOnly = 0x02,
        ReadWrite = 0x03,
        Append    = 0x04,
        Truncate  = 0x08
    };

    enum Permission {
        ReadOwner   = 0x4000,
        WriteOwner  = 0x2000,
        ReadUser    = 0x0400,
        WriteUser   = 0x0200,
        ReadGroup   = 0x0040,
        WriteGroup  = 0x0020,
        ReadOther   = 0x0004,
        WriteOther  = 0x0002,
        DefaultPerm = 0x0001
    };

    File() : fp_(nullptr) {}
    explicit File(const String & path) : path_(path), fp_(nullptr) {}
    ~File() { close(); }

    File(File && other) : path_(std::move(other.path_)), fp_(other.fp_) { other.fp_ = nullptr; }

    void setFileName(const String & path) { path_ = path; }

    bool open(int fd, int mode) {
        if (fp_) return false;
        const char * m = (mode & Append) ? "a" : ((mode & WriteOnly) ? "w" : "r");
        fp_ = fdopen(fd, m);
        return fp_ != nullptr;
    }

    bool open(int mode);

    void close() {
        if (fp_) { fclose(fp_); fp_ = nullptr; }
    }

    bool isOpen() const { return fp_ != nullptr; }

    int64 write(const ByteArray & data) {
        if (!fp_) return -1;
        return (int64)fwrite(data.constCharPtr(), 1, data.size(), fp_);
    }
    int64 write(const char * data, size_t len) {
        if (!fp_) return -1;
        return (int64)fwrite(data, 1, len, fp_);
    }

    void flush() { if (fp_) fflush(fp_); }

    ByteArray readAll();

    static ByteArray read    (const String & path);
    static String    readUtf8(const String & path);
    static bool      write   (const String & path, const ByteArray & data, int perm = DefaultPerm);

    bool setPermissions(int perms) {
        if (path_.isEmpty()) return false;
        mode_t m = 0;
        if (perms & ReadOwner)  m |= S_IRUSR;
        if (perms & WriteOwner) m |= S_IWUSR;
        if (perms & ReadUser)   m |= S_IRUSR;
        if (perms & WriteUser)  m |= S_IWUSR;
        if (perms & ReadGroup)  m |= S_IRGRP;
        if (perms & WriteGroup) m |= S_IWGRP;
        if (perms & ReadOther)  m |= S_IROTH;
        if (perms & WriteOther) m |= S_IWOTH;
        return chmod(path_.toStdString().c_str(), m) == 0;
    }

    String fileName() const { return path_; }
    String errorString() const { return String(strerror(errno)); }

    static bool exists(const String & path) {
        struct stat st;
        return stat(path.toStdString().c_str(), &st) == 0;
    }

    static void registerData(const String & file, const uint8 * data, size_t size);

private:
    String path_;
    FILE * fp_;
};

} // namespace
