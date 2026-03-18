/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serialize.h>

class Test1
{
public:
    int a;
    int b;

    bool operator!=(const Test1 & rhs) const { return a != rhs.a || b != rhs.b; }
    bool isNull() const { return a == 0 && b == 0; }

    template<typename T>
    void serialize(T & ser) const
    {
        ser << a << b;
    }
    template<typename T>
    void deserialize(T & ser)
    {
        ser >> a >> b;
    }
};

SERIALIZE_CLASS_USE_NULL(Test1)

class Test2
{
public:
    Test1 t1;
    int a;

    bool operator!=(const Test2 & rhs) const { return t1 != rhs.t1 || a != rhs.a; }

    template<typename T>
    void serialize(T & ser) const
    {
        ser << t1 << a;
    }
    template<typename T>
    void deserialize(T & ser)
    {
        ser >> t1 >> a;
    }
};
