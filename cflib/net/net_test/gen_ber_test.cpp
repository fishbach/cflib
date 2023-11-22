/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/net_test/gentest.h>
#include <cflib/net/net_test/test/gentest2.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Gen_BER_Test: public QObject
{
    Q_OBJECT
private slots:

    void serialize()
    {
        GenTest1 gt1;
        gt1.a = 3;
        gt1.c = 4;
        gt1.d = "xy";
        gentest::GenTest3 gt3;
        gt3.a = 5;
        gt3.c = 6;
        gt3.d = "ab";
        gt3.e = 7;
        gt3.f = 8;
        BERSerializer ser;
        ser << gt1 << gt3;
        QCOMPARE(ser.data(), QByteArray::fromHex(
            "                    E10A C20103 C40104 C5027879"
            "E219 C10500E4440B6F E20A C20105 C40106 C5026162 C30107 C40108"
        ));
    }

    void deserialize()
    {
        BERDeserializer ser(QByteArray::fromHex(
            "                    E10A C20103 C40104 C5027879"
            "E219 C10500E4440B6F E20A C20105 C40106 C5026162 C30107 C40108"
        ));
        GenTest1 gt1;
        gentest::GenTest3 gt3;
        ser >> gt1 >> gt3;
        QCOMPARE(gt1.a, 3);
        QCOMPARE(gt1.c, 4);
        QCOMPARE(gt1.d, QString("xy"));
        QCOMPARE(gt3.a, 5);
        QCOMPARE(gt3.c, 6);
        QCOMPARE(gt3.d, QString("ab"));
        QCOMPARE(gt3.e, 7);
        QCOMPARE(gt3.f, 8);
    }

    void template_ser()
    {
        gentest::gentest2::GenTest4 gt4;
        gt4 << "AB";
        gt4 << "";
        gt4.a = 7;
        gt4.b << 13 << 17;
        BERSerializer ser;
        ser << gt4;
        QCOMPARE(ser.data(), QByteArray::fromHex(
            "E11A"
            "C10500FEDBD07E"
            "E206 C0024142 C000"
            "C30107"
            "E406 C0010D C00111"
        ));
    }

    void template_deser()
    {
        BERDeserializer ser(QByteArray::fromHex(
            "E11A"
            "C10500FEDBD07E"
            "E206 C0024142 C000"
            "C30107"
            "E406 C0010D C00111"
        ));
        gentest::gentest2::GenTest4 gt4;
        ser >> gt4;
        QCOMPARE(gt4.size(), 2);
        QCOMPARE(gt4[0], QString("AB"));
        QVERIFY(!gt4[1].isNull());
        QVERIFY(gt4[1].isEmpty());
        QCOMPARE(gt4.a, 7);
        QCOMPARE(gt4.b.size(), 2);
        QCOMPARE(gt4.b[0], 13);
        QCOMPARE(gt4.b[1], 17);
    }

};
#include "gen_ber_test.moc"
ADD_TEST(Gen_BER_Test)
