/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/net_test/gentest.h>
#include <cflib/net/net_test/test/gentest2.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Gen_BER_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Gen_BER_Test *>(this);
        return {
            {"serialize",       [self]() { self->serialize(); }},
            {"deserialize",     [self]() { self->deserialize(); }},
            {"template_ser",    [self]() { self->template_ser(); }},
            {"template_deser",  [self]() { self->template_deser(); }}
        };
    }

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
        TCOMPARE(ser.data(), ByteArray::fromHex(
            "                    E10A C20103 C40104 C5027879"
            "E219 C10500E4440B6F E20A C20105 C40106 C5026162 C30107 C40108"
        ));
    }

    void deserialize()
    {
        BERDeserializer ser(ByteArray::fromHex(
            "                    E10A C20103 C40104 C5027879"
            "E219 C10500E4440B6F E20A C20105 C40106 C5026162 C30107 C40108"
        ));
        GenTest1 gt1;
        gentest::GenTest3 gt3;
        ser >> gt1 >> gt3;
        TCOMPARE(gt1.a, 3);
        TCOMPARE(gt1.c, 4);
        TCOMPARE(gt1.d, String("xy"));
        TCOMPARE(gt3.a, 5);
        TCOMPARE(gt3.c, 6);
        TCOMPARE(gt3.d, String("ab"));
        TCOMPARE(gt3.e, 7);
        TCOMPARE(gt3.f, 8);
    }

    void template_ser()
    {
        gentest::gentest2::GenTest4 gt4;
        gt4.push_back(String("AB"));
        gt4.push_back(String(""));
        gt4.a = 7;
        gt4.b << 13 << 17;
        BERSerializer ser;
        ser << gt4;
        TCOMPARE(ser.data(), ByteArray::fromHex(
            "E11A"
            "C10500FEDBD07E"
            "E206 C0024142 C000"
            "C30107"
            "E406 C0010D C00111"
        ));
    }

    void template_deser()
    {
        BERDeserializer ser(ByteArray::fromHex(
            "E11A"
            "C10500FEDBD07E"
            "E206 C0024142 C000"
            "C30107"
            "E406 C0010D C00111"
        ));
        gentest::gentest2::GenTest4 gt4;
        ser >> gt4;
        TCOMPARE((int)gt4.size(), 2);
        TCOMPARE(gt4[0], String("AB"));
        TVERIFY(!gt4[1].isNull());
        TVERIFY(gt4[1].isEmpty());
        TCOMPARE(gt4.a, 7);
        TCOMPARE((int)gt4.b.size(), 2);
        TCOMPARE(gt4.b[0], 13);
        TCOMPARE(gt4.b[1], 17);
    }
};

ADD_TEST(Gen_BER_Test)
