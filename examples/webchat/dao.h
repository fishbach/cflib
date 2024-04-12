/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
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
    QString name;
    quint32 number;
    typedef QList<quint32> List;
    typedef QPair<List, QDateTime> Pair;
    QPair<quint8, Pair> pair;
};

class Dao2
{
    SERIALIZE_CLASS
    SERIALIZE_IS_BASE(Dao2)
public serialized:
    Dao dao;
    QList<int> numbers;
    double f;
};

class Dao3 : public Dao2
{
    SERIALIZE_CLASS
    SERIALIZE_BASE(Dao3)
public serialized:
    QDateTime timestamp;
};
