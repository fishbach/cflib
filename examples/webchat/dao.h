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
    typedef List<cfuint32> IList;
    typedef Pair<IList, CFDateTime> LDPair;
    Pair<cfuint8, LDPair> pair;
};

class Dao2
{
    SERIALIZE_CLASS
    SERIALIZE_IS_BASE(Dao2)
public serialized:
    Dao dao;
    List<int> numbers;
    double f;
};

class Dao3 : public Dao2
{
    SERIALIZE_CLASS
    SERIALIZE_BASE(Dao3)
public serialized:
    CFDateTime timestamp;
};
