/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <doctest/doctest.h>

namespace doctest {

template<>
struct StringMaker<cflib::base::String>
{
    static String convert(const cflib::base::String & value)
    {
        if (value.isNull()) return "<null>";
        return String(value.c_str(), (String::size_type)value.byteSize());
    }
};

template<>
struct StringMaker<cflib::base::ByteArray>
{
    static String convert(const cflib::base::ByteArray & value)
    {
        if (value.isNull()) return "<null>";
        const cflib::base::ByteArray hex = value.toHex();
        return String(hex.constData(), (String::size_type)hex.size());
    }
};

}
