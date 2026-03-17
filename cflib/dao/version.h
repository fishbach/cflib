/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serialize.h>

#ifdef major
    #undef major
#endif
#ifdef minor
    #undef minor
#endif

namespace cflib { namespace dao {

class Version
{
    SERIALIZE_CLASS
public:
    Version(cfuint major = 0, cfuint minor = 0, cfuint revision = 0, const CFString & patchLevel = CFString()) :
        major(major), minor(minor), revision(revision), patchLevel(patchLevel) {}

    bool isNull() const { return major == 0 && minor == 0 && revision == 0 && patchLevel.isNull(); }

    bool operator==(const Version & rhs) const { return
        major      == rhs.major &&
        minor      == rhs.minor &&
        revision   == rhs.revision &&
        patchLevel == rhs.patchLevel;
    }
    bool operator!=(const Version & rhs) const { return !operator==(rhs); }
    bool operator>=(const Version & rhs) const {
        if (major >= rhs.major) return true;
        if (minor >= rhs.minor) return true;
        if (revision >= rhs.revision) return true;
        return patchLevel >= rhs.patchLevel;
    }
    bool operator<(const Version & rhs) const { return !operator>=(rhs); }
    bool operator>(const Version & rhs) const { return !rhs.operator>=(*this); }
    bool operator<=(const Version & rhs) const { return rhs.operator>=(*this); }

    CFString toString() const;

    static Version current() { return current_; }
    static void setCurrent(const Version & version);
    static void setCurrent(cfuint major = 0, cfuint minor = 0, cfuint revision = 0, const CFString & patchLevel = CFString());

public serialized:
    cfuint major;
    cfuint minor;
    cfuint revision;
    CFString patchLevel;

private:
    static Version current_;
};

}}    // namespace
