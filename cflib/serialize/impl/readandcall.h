/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serializeber.h>

#include <type_traits>

namespace cflib { namespace serialize { namespace impl {

template <typename... P> struct ReadAndCall;

template <typename P, typename... PN>
struct ReadAndCall<P, PN...>
{
    template<typename F, typename... A>
    void operator()(BERDeserializer & deser, F func, A... a)
    {
        typename std::decay<P>::type p;
        deser >> p;
        ReadAndCall<PN...>()(deser, func, std::forward<A>(a)..., std::move(p));
    }
};

template <>
struct ReadAndCall<>
{
    template<typename F, typename... A>
    void operator()(BERDeserializer &, F func, A... a)
    {
        func(std::forward<A>(a)...);
    }
};

}}}    // namespace
