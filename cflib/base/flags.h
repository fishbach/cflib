/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/types.h>

namespace cflib::base {

template<typename E>
class Flags
{
public:
    Flags() noexcept : val_(0) {}
    Flags(E flag) noexcept : val_((cfuint32)flag) {}
    explicit Flags(cfuint32 v) noexcept : val_(v) {}

    cfuint32 toInt() const noexcept { return val_; }
    bool isNull() const noexcept { return val_ == 0; }

    Flags operator|(Flags o) const noexcept { return Flags(val_ | o.val_); }
    Flags operator&(Flags o) const noexcept { return Flags(val_ & o.val_); }
    Flags operator^(Flags o) const noexcept { return Flags(val_ ^ o.val_); }
    Flags operator~()          const noexcept { return Flags(~val_); }
    Flags & operator|=(Flags o) noexcept { val_ |= o.val_; return *this; }
    Flags & operator&=(Flags o) noexcept { val_ &= o.val_; return *this; }

    bool operator==(Flags o) const noexcept { return val_ == o.val_; }
    bool operator!=(Flags o) const noexcept { return val_ != o.val_; }

    explicit operator bool() const noexcept { return val_ != 0; }
    explicit operator cfuint32() const noexcept { return val_; }
    explicit operator int() const noexcept { return (int)val_; }

private:
    cfuint32 val_;
};

} // namespace
