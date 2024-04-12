/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/rmiservice.h>
#include <cflib/net/rsig.h>
#include <cflib/serialize/serialize.h>

class GenTestRMI : public cflib::net::RMIServiceBase
{
    SERIALIZE_CLASS
public:
    void f1(int) {}
    int f2() { return 4; }

rmi:
    void f3(int, QString) {}
    QList<int> f4() { return QList<int>(); }
    int f5(int x, int y);
    void f6();

public:
    int x;
};

class GenTest1
{
    SERIALIZE_CLASS
    SERIALIZE_IS_BASE(GenTest1)
public:
    void f1(int) {}
    int f2() { return 4; }

public:
    int x;

public serialized:
    int a;
    SERIALIZE_SKIP(int b)
    int c;
    QString d;

public:
    int y;
};

inline int GenTestRMI::f5(int x, int y)
{
    return x + y;
}

inline void GenTestRMI::f6()
{
}

class GenTest2
{
    SERIALIZE_CLASS
public serialized:
    GenTest1 a;
    int b;
};

namespace gentest {

class GenTest3 : public GenTest1
{
    SERIALIZE_CLASS
    SERIALIZE_BASE(GenTest3)
public serialized:
    int e;

    class Inner1 {
        SERIALIZE_CLASS
    public serialized:
        int a;
    };
    struct Inner2 {
        SERIALIZE_CLASS
    };
    struct Inner3 {};

    int f;

private:
    int z2;

public:
    int func() const { return z2; }
};

namespace gentest2 {

class GenTest4 : public QList<QString>
{
    SERIALIZE_CLASS
    SERIALIZE_STDBASE(GenTest4)
public serialized:
    int a;
    QList<int> b;
    QList<GenTest2> c;
};

}

}

class GenTest5
{
public serialized:
    int a;
};
