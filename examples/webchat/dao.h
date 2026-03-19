/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serialize.h>

class Dao
{
    SERIALIZE_CLASS
public serialized:
    String name;
    cfuint32 number;
    typedef CFList<cfuint32> List;
    typedef CFPair<List, CFDateTime> Pair;
    CFPair<cfuint8, Pair> pair;
};

class Dao2
{
    SERIALIZE_CLASS
    SERIALIZE_IS_BASE(Dao2)
public serialized:
    Dao dao;
    CFList<int> numbers;
    double f;
};

class Dao3 : public Dao2
{
    SERIALIZE_CLASS
    SERIALIZE_BASE(Dao3)
public serialized:
    CFDateTime timestamp;
};
