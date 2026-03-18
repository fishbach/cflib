/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/types.h>

// Unicode codepoint, backed by char32_t
class CFChar
{
public:
    CFChar() noexcept : cp_(0) {}
    CFChar(char32_t cp) noexcept : cp_(cp) {}
    CFChar(char c) noexcept : cp_((cfuint32)(cfuint8)c) {}

    char32_t unicode() const noexcept { return cp_; }

    bool isNull()  const noexcept { return cp_ == 0; }
    bool isLetter()  const noexcept { return (cp_ >= 'A' && cp_ <= 'Z') || (cp_ >= 'a' && cp_ <= 'z'); }
    bool isDigit()   const noexcept { return cp_ >= '0' && cp_ <= '9'; }
    bool isSpace()   const noexcept { return cp_ == ' ' || cp_ == '\t' || cp_ == '\n' || cp_ == '\r'; }
    bool isLower()   const noexcept { return cp_ >= 'a' && cp_ <= 'z'; }
    bool isUpper()   const noexcept { return cp_ >= 'A' && cp_ <= 'Z'; }

    CFChar toLower() const noexcept {
        if (cp_ >= 'A' && cp_ <= 'Z') return CFChar((char32_t)(cp_ + 32));
        return *this;
    }
    CFChar toUpper() const noexcept {
        if (cp_ >= 'a' && cp_ <= 'z') return CFChar((char32_t)(cp_ - 32));
        return *this;
    }

    bool operator==(const CFChar & o) const noexcept { return cp_ == o.cp_; }
    bool operator!=(const CFChar & o) const noexcept { return cp_ != o.cp_; }
    bool operator< (const CFChar & o) const noexcept { return cp_ <  o.cp_; }

private:
    char32_t cp_;
};
