/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::net::fileserver {

class FileServerBase
{
public:
    FileServerBase(const String & path);

    void exportTo(const String & dest) const;

protected:
    String parseHtml(const String & fullPath, bool isPart, const String & path,
        const StringList & params = StringList()) const;

private:
    void exportDir(const String & fullPath, const String & path, const String & dest) const;

protected:
    const String path_;
    const ByteArray eTag_;

private:
    const Regex elementRE_;
};

} // namespace
