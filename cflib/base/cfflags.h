/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/types.h>

template<typename E>
class CFFlags
{
public:
    CFFlags() noexcept : val_(0) {}
    CFFlags(E flag) noexcept : val_((cfuint32)flag) {}
    explicit CFFlags(cfuint32 v) noexcept : val_(v) {}

    cfuint32 toInt() const noexcept { return val_; }
    bool isNull() const noexcept { return val_ == 0; }

    CFFlags operator|(CFFlags o) const noexcept { return CFFlags(val_ | o.val_); }
    CFFlags operator&(CFFlags o) const noexcept { return CFFlags(val_ & o.val_); }
    CFFlags operator^(CFFlags o) const noexcept { return CFFlags(val_ ^ o.val_); }
    CFFlags operator~()          const noexcept { return CFFlags(~val_); }
    CFFlags & operator|=(CFFlags o) noexcept { val_ |= o.val_; return *this; }
    CFFlags & operator&=(CFFlags o) noexcept { val_ &= o.val_; return *this; }

    bool operator==(CFFlags o) const noexcept { return val_ == o.val_; }
    bool operator!=(CFFlags o) const noexcept { return val_ != o.val_; }

    explicit operator bool() const noexcept { return val_ != 0; }
    explicit operator cfuint32() const noexcept { return val_; }
    explicit operator int() const noexcept { return (int)val_; }

private:
    cfuint32 val_;
};
