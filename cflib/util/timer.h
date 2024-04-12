/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/functor.h>

namespace cflib { namespace util {

class Timer
{
public:
    static void singleShot(double afterSecs, const Functor * func);

    template<typename C>
    static void singleShot(double afterSecs, C * obj, void (C::*func)())
    {
        singleShot(afterSecs, new util::Functor0<C>(obj, func));
    }

    template<typename C>
    static void singleShot(double afterSecs, const C * obj, void (C::*func)() const)
    {
        singleShot(afterSecs, new util::Functor0C<C>(obj, func));
    }

private:
    static void timeout(int revents, void * arg);
};

}}    // namespace
