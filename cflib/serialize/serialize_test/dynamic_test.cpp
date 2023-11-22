/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/serialize_test/dynamic.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Dynamic_Test: public QObject
{
    Q_OBJECT
private slots:

    void serializeNull()
    {
        DynamicUse in;
        in.y = 23;
        BERSerializer ser;
        ser << in;

        DynamicUse out;
        BERDeserializer deser(ser.data());
        deser >> out;

        QVERIFY(in.y == out.y);
        QVERIFY(in.d.isNull());
        QVERIFY(in.e.isEmpty());
        QVERIFY(in.z == out.z);
    }

    void serialize()
    {
        DynamicUse in;
        in.y = 23;
        DynamicA * dynA = new DynamicA();
        dynA->a = 45;
        in.d.reset(dynA);
        DynamicB * dynB = new DynamicB();
        dynB->b = 123.45;
        in.e << QSharedPointer<DynamicBase>(dynB);
        BERSerializer ser;
        ser << in;

        DynamicUse out;
        BERDeserializer deser(ser.data());
        deser >> out;

        QCOMPARE(out.y, in.y);
        QVERIFY(!in.d.isNull());
        QCOMPARE(in.d.dynamicCast<DynamicA>()->a, 45);
        QCOMPARE(in.e.size(), 1);
        QCOMPARE(in.e.first().dynamicCast<DynamicB>()->b, 123.45);
        QCOMPARE(out.z, in.z);
    }

};
#include "dynamic_test.moc"
ADD_TEST(Dynamic_Test)
