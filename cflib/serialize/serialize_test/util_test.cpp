/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/util.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Util_Test: public QObject
{
    Q_OBJECT
private slots:

    void test_getTLVLength()
    {
        quint64 tag;
        int tagLen;
        int lengthSize;
        QCOMPARE(getTLVLength(QByteArray::fromHex(""            ), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c1"          ), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c100"        ), tag, tagLen, lengthSize),  0); QCOMPARE(lengthSize, 1);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c18100"      ), tag, tagLen, lengthSize),  0); QCOMPARE(lengthSize, 2);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c180"        ), tag, tagLen, lengthSize), -2);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c101"        ), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c10100"      ), tag, tagLen, lengthSize),  1); QCOMPARE(lengthSize, 1);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c1810100"    ), tag, tagLen, lengthSize),  1); QCOMPARE(lengthSize, 2);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c1847FFFFFF9"), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c1847FFFFFFA"), tag, tagLen, lengthSize), -3);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c18480000000"), tag, tagLen, lengthSize), -3);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c188"        ), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(QByteArray::fromHex("c189"        ), tag, tagLen, lengthSize), -3);

        getTLVLength(QByteArray::fromHex("c000"    ), tag, tagLen, lengthSize); QCOMPARE(tag, Q_UINT64_C(  0)); QCOMPARE(tagLen, 1);
        getTLVLength(QByteArray::fromHex("c100"    ), tag, tagLen, lengthSize); QCOMPARE(tag, Q_UINT64_C(  1)); QCOMPARE(tagLen, 1);
        getTLVLength(QByteArray::fromHex("DE00"    ), tag, tagLen, lengthSize); QCOMPARE(tag, Q_UINT64_C( 30)); QCOMPARE(tagLen, 1);
        getTLVLength(QByteArray::fromHex("DF1F00"  ), tag, tagLen, lengthSize); QCOMPARE(tag, Q_UINT64_C( 31)); QCOMPARE(tagLen, 2);
        getTLVLength(QByteArray::fromHex("DF7F00"  ), tag, tagLen, lengthSize); QCOMPARE(tag, Q_UINT64_C(127)); QCOMPARE(tagLen, 2);
        getTLVLength(QByteArray::fromHex("DF810000"), tag, tagLen, lengthSize); QCOMPARE(tag, Q_UINT64_C(128)); QCOMPARE(tagLen, 3);
        getTLVLength(QByteArray::fromHex("DF810100"), tag, tagLen, lengthSize); QCOMPARE(tag, Q_UINT64_C(129)); QCOMPARE(tagLen, 3);
    }

    void test_toByteArray()
    {
        QCOMPARE(toByteArray(0, 0), QByteArray::fromHex("C08100"));
        QCOMPARE(toByteArray(0), QByteArray::fromHex(""));

        QCOMPARE(toByteArray(1), QByteArray::fromHex("C10101"));
        QCOMPARE(toByteArray(-1), QByteArray::fromHex("C101FF"));
        QCOMPARE(toByteArray("bla"), QByteArray::fromHex("C103626C61"));

        QCOMPARE(toByteArray(1,    30), QByteArray::fromHex("DE0101"));
        QCOMPARE(toByteArray(1,    31), QByteArray::fromHex("DF1F0101"));
        QCOMPARE(toByteArray(1,   127), QByteArray::fromHex("DF7F0101"));
        QCOMPARE(toByteArray(1,   128), QByteArray::fromHex("DF81000101"));
        QCOMPARE(toByteArray(1,   129), QByteArray::fromHex("DF81010101"));
        QCOMPARE(toByteArray(1,   255), QByteArray::fromHex("DF817F0101"));
        QCOMPARE(toByteArray(1,   256), QByteArray::fromHex("DF82000101"));
        QCOMPARE(toByteArray(1, 16383), QByteArray::fromHex("DFFF7F0101"));
        QCOMPARE(toByteArray(1, 16384), QByteArray::fromHex("DF8180000101"));

        QCOMPARE(toByteArray(QByteArray(),   3), QByteArray::fromHex(""));
        QCOMPARE(toByteArray(QByteArray(""), 3), QByteArray::fromHex("C300"));
    }

    void test_fromByteArray()
    {
        QCOMPARE(fromByteArray<int>(QByteArray::fromHex("")), 0);
        QCOMPARE(fromByteArray<int>(QByteArray::fromHex("C000")), 0);
        QCOMPARE(fromByteArray<int>(QByteArray::fromHex("C08100")), 0);
        QCOMPARE(fromByteArray<int>(QByteArray::fromHex("C10101")), 1);
        QCOMPARE(fromByteArray<int>(QByteArray::fromHex("C101FF")), -1);
        QCOMPARE(fromByteArray<QString>(QByteArray::fromHex("")), QString());
        QCOMPARE(fromByteArray<QString>(QByteArray::fromHex("C000")), QString());
        QVERIFY(!fromByteArray<QString>(QByteArray::fromHex("C000")).isNull());
        QCOMPARE(fromByteArray<QString>(QByteArray::fromHex("C08100")), QString());
        QVERIFY(fromByteArray<QString>(QByteArray::fromHex("C08100")).isNull());
        QCOMPARE(fromByteArray<QString>(QByteArray::fromHex("C103626C61")), QString("bla"));
    }

    void test_sizes()
    {
        QCOMPARE((int)sizeof(float      ),  4);
        QCOMPARE((int)sizeof(double     ),  8);
        QCOMPARE((int)sizeof(long double), 16);
    }

    void test_readAndCall()
    {
        BERSerializer ser;
        ser << 34 << "bla";
        BERDeserializer deser(ser.data());
        int i = 0;
        QString s;
        readAndCall<int, const QString &>(deser, [&](int pi, const QString & ps) { i = pi; s = ps; });
        QCOMPARE(i, 34);
        QCOMPARE(s, QString("bla"));
    }

};
#include "util_test.moc"
ADD_TEST(Util_Test)
