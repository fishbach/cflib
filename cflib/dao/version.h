/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
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

namespace cflib::dao {

class Version
{
    SERIALIZE_CLASS
public:
    Version(uint major = 0, uint minor = 0, uint revision = 0, const String & patchLevel = String()) :
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

    String toString() const;

    static Version current() { return current_; }
    static void setCurrent(const Version & version);
    static void setCurrent(uint major = 0, uint minor = 0, uint revision = 0, const String & patchLevel = String());

public serialized:
    uint major;
    uint minor;
    uint revision;
    String patchLevel;

private:
    static Version current_;
};

} // namespace
