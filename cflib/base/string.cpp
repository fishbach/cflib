/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "string.h"

namespace cflib::base {

String String::simplified() const
{
    std::string r;
    bool lastWasSpace = true;
    for (char c : d->data) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (!lastWasSpace) { r += ' '; lastWasSpace = true; }
        } else { r += c; lastWasSpace = false; }
    }
    if (!r.empty() && r.back() == ' ') r.pop_back();
    return String(std::move(r));
}

} // namespace
