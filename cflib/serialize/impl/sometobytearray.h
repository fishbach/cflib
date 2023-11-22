/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serializeber.h>

namespace cflib { namespace serialize { namespace impl {

template <size_t I, typename... P> struct SomeToByteArray;

template <size_t I, typename P, typename... PN>
struct SomeToByteArray<I, P, PN...>
{
    void operator()(BERSerializer & ser, size_t start, size_t count, P p, PN... pn)
    {
        if (I >= start + count) return;
        if (I < start) ser << Placeholder();
        else           ser << p;
        SomeToByteArray<I + 1, PN...>()(ser, start, count, pn...);
    }
};

template <size_t I>
struct SomeToByteArray<I>
{
    void operator()(BERSerializer &, size_t, size_t) {}
};

template <typename... P> struct ToByteArray;

template <typename P, typename... PN>
struct ToByteArray<P, PN...>
{
    void operator()(BERSerializer & ser, P p, PN... pn)
    {
        ser << p;
        ToByteArray<PN...>()(ser, std::forward<PN>(pn)...);
    }
};

template <>
struct ToByteArray<>
{
    void operator()(BERSerializer &) {}
};

}}}    // namespace
