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
    Flags() : val_(0) {}
    Flags(E flag) : val_((uint32)flag) {}
    explicit Flags(uint32 v) : val_(v) {}

    uint32 toInt() const { return val_; }
    bool isNull() const { return val_ == 0; }

    Flags operator|(Flags o) const { return Flags(val_ | o.val_); }
    Flags operator&(Flags o) const { return Flags(val_ & o.val_); }
    Flags operator^(Flags o) const { return Flags(val_ ^ o.val_); }
    Flags operator~()          const { return Flags(~val_); }
    Flags & operator|=(Flags o) { val_ |= o.val_; return *this; }
    Flags & operator&=(Flags o) { val_ &= o.val_; return *this; }

    bool operator==(Flags o) const { return val_ == o.val_; }
    bool operator!=(Flags o) const { return val_ != o.val_; }

    explicit operator bool() const { return val_ != 0; }
    explicit operator uint32() const { return val_; }
    explicit operator int() const { return (int)val_; }

private:
    uint32 val_;
};

} // namespace
