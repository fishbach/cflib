/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/serialize_test/test.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

namespace {

template<typename T>
bool checkSer(T val, const char * hex)
{
    BERSerializer ser;
    ser << val;
    const CFByteArray expected = CFByteArray::fromHex(hex);
    if (ser.data() != expected) {
        fprintf(stdout, "serialized hex differs:\nis       : %s\nexpected : %s\n",
            ser.data().toHex().data(), expected.toHex().data());
        return false;
    }
    return true;
}

template<typename T>
bool checkDeser(T val, const char * hex)
{
    const CFByteArray expected = CFByteArray::fromHex(hex);
    BERDeserializer deser(expected);
    T test; deser >> test;
    if (test != val) {
        fprintf(stdout, "deserialized values differ\n");
        return false;
    }
    return true;
}

template<typename T>
bool checkDeserNull(T val, const char * hex)
{
    const CFByteArray expected = CFByteArray::fromHex(hex);
    BERDeserializer deser(expected);
    T test; deser >> test;
    if (test.isNull() != val.isNull()) {
        fprintf(stdout, "deserialized isNull differs: is=%d expected=%d\n",
            (int)test.isNull(), (int)val.isNull());
        return false;
    }
    return true;
}

template<typename T>
bool checkSerDeser(T val, const char * hex)
{
    return checkSer(val, hex) && checkDeser(val, hex);
}

template<typename T>
bool checkSerDeserNull(T val, const char * hex)
{
    return checkSer(val, hex) && checkDeser(val, hex) && checkDeserNull(val, hex);
}

bool testBigTag(int tagNr, const char * hex)
{
    bool retval = true;
    BERSerializer ser;
    for (int i = 0 ; i < tagNr - 1 ; ++i) ser << Placeholder();
    ser << 0x42;
    const CFByteArray expected = CFByteArray::fromHex(hex) + CFByteArray::fromHex("0142");
    if (ser.data() != expected) {
        fprintf(stdout, "serialized hex differs:\nis       : %s\nexpected : %s\n",
            ser.data().toHex().data(), expected.toHex().data());
        retval = false;
    }
    BERDeserializer deser(expected);
    for (int i = 0 ; i < tagNr - 1 ; ++i) deser >> Placeholder();
    int test; deser >> test;
    if (test != 0x42) {
        fprintf(stdout, "deserialized values differ: is=%d expected=%d\n", test, 0x42);
        return false;
    }
    return retval;
}

}

class SerializeBER_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<SerializeBER_Test *>(this);
        return {
            {"integer",    [self]() { self->integer(); }},
            {"nullInList", [self]() { self->nullInList(); }},
            {"string",     [self]() { self->string(); }},
            {"many",       [self]() { self->many(); }},
            {"bigTag",     [self]() { self->bigTag(); }},
            {"object",     [self]() { self->object(); }},
            {"lists",      [self]() { self->lists(); }},
            {"maps",       [self]() { self->maps(); }}
        };
    }

    void integer()
    {
        QVERIFY(checkSerDeser<bool      >( false, ""          ));
        QVERIFY(checkSerDeser<bool      >(  true, "c10101"    ));
        QVERIFY(checkSerDeser<cfuint8   >(     0, ""          ));
        QVERIFY(checkSerDeser<cfuint8   >(     1, "c10101"    ));
        QVERIFY(checkSerDeser<cfuint8   >(   127, "c1017f"    ));
        QVERIFY(checkSerDeser<cfuint8   >(   255, "c10200ff"  ));
        QVERIFY(checkSerDeser<cfint8    >(     0, ""          ));
        QVERIFY(checkSerDeser<cfint8    >(     1, "c10101"    ));
        QVERIFY(checkSerDeser<cfint8    >(   127, "c1017f"    ));
        QVERIFY(checkSerDeser<cfint8    >(    -1, "c101ff"    ));
        QVERIFY(checkSerDeser<cfint8    >(    -2, "c101fe"    ));
        QVERIFY(checkSerDeser<cfint8    >(  -128, "c10180"    ));
        QVERIFY(checkSerDeser<cfuint16  >(     0, ""          ));
        QVERIFY(checkSerDeser<cfuint16  >(     1, "c10101"    ));
        QVERIFY(checkSerDeser<cfuint16  >( 32767, "c1027fff"  ));
        QVERIFY(checkSerDeser<cfuint16  >( 65535, "c10300ffff"));
        QVERIFY(checkSerDeser<cfint16   >(     0, ""          ));
        QVERIFY(checkSerDeser<cfint16   >(     1, "c10101"    ));
        QVERIFY(checkSerDeser<cfint16   >(   127, "c1017f"    ));
        QVERIFY(checkSerDeser<cfint16   >(   128, "c1020080"  ));
        QVERIFY(checkSerDeser<cfint16   >( 32767, "c1027fff"  ));
        QVERIFY(checkSerDeser<cfint16   >(    -1, "c101ff"    ));
        QVERIFY(checkSerDeser<cfint16   >(    -2, "c101fe"    ));
        QVERIFY(checkSerDeser<cfint16   >(  -128, "c10180"    ));
        QVERIFY(checkSerDeser<cfint16   >(  -129, "c102ff7f"  ));
        QVERIFY(checkSerDeser<cfint16   >(-32767, "c1028001"  ));
        QVERIFY(checkSerDeser<cfint16   >(-32768, "c1028000"  ));
        QVERIFY(checkSerDeser<cfuint64  >(     0, ""          ));
        QVERIFY(checkSerDeser<cfuint64  >(     1, "c10101"    ));
        QVERIFY(checkSerDeser<cfuint64  >(   128, "c1020080"  ));
        QVERIFY(checkSerDeser<cfint64   >(    -1, "c101ff"    ));
        QVERIFY(checkSerDeser<cfint64   >(    -2, "c101fe"    ));
        QVERIFY(checkSerDeser<cfint64   >(  -128, "c10180"    ));
        QVERIFY(checkSerDeser<cfint64   >(  -129, "c102ff7f"  ));

        QVERIFY(checkSerDeser<cfuint64>((cfuint64)UINT64_C(0x7fffffffffffffff), "c1087fffffffffffffff"));
        QVERIFY(checkSerDeser<cfuint64>((cfuint64)UINT64_C(0x8000000000000000), "c109008000000000000000"));
        QVERIFY(checkSerDeser<cfuint64>((cfuint64)UINT64_C(0xfffffffffffffffe), "c10900fffffffffffffffe"));
        QVERIFY(checkSerDeser<cfuint64>((cfuint64)UINT64_C(0xffffffffffffffff), "c10900ffffffffffffffff"));

        QVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C(   36028797018963967),   "c1077fffffffffffff"));
        QVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C(   36028797018963968),   "c1080080000000000000"));
        QVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C( 9223372036854775807),   "c1087fffffffffffffff"));
        QVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C(-9223372036854775807),   "c1088000000000000001"));
        QVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C(-9223372036854775807)-1, "c1088000000000000000"));
    }

    void nullInList()
    {
        QVERIFY(checkSerDeser<CFList<bool      >>(CFList<bool      >{false}, "e103c08100"));
        QVERIFY(checkSerDeser<CFList<cfuint8   >>(CFList<cfuint8   >{0},     "e103c08100"));
        QVERIFY(checkSerDeser<CFList<cfint8    >>(CFList<cfint8    >{0},     "e103c08100"));
        QVERIFY(checkSerDeser<CFList<cfuint16  >>(CFList<cfuint16  >{0},     "e103c08100"));
        QVERIFY(checkSerDeser<CFList<cfuint64  >>(CFList<cfuint64  >{0},     "e103c08100"));

        QVERIFY(checkSer<CFList<const char *>>(CFList<const char *>{nullptr},          "e103c08100"));
        QVERIFY(checkSerDeser<CFList<CFByteArray>>(CFList<CFByteArray>{CFByteArray()}, "e103c08100"));
        QVERIFY(checkSerDeser<CFList<CFString   >>(CFList<CFString   >{CFString()},    "e103c08100"));
        QVERIFY(checkSerDeser<CFList<CFChar     >>(CFList<CFChar     >{CFChar()},      "e103c08100"));
    }

    void string()
    {
        QVERIFY(checkSer<const char *>(nullptr, ""));
        QVERIFY(checkSer<const char *>("",      "c100"));
        QVERIFY(checkSer<const char *>("X",     "c10158"));
        QVERIFY(checkSer<const char *>("XY",    "c1025859"));

        QVERIFY(checkSerDeserNull<CFByteArray>(CFByteArray(),     ""));
        QVERIFY(checkSerDeserNull<CFByteArray>(CFByteArray(""),   "c100"));
        QVERIFY(checkSerDeserNull<CFByteArray>(CFByteArray("X"),  "c10158"));
        QVERIFY(checkSerDeserNull<CFByteArray>(CFByteArray("XY"), "c1025859"));
        QVERIFY(checkSerDeserNull<CFByteArray>(CFByteArray::fromHex("003132"), "c103 003132"));
        QVERIFY(checkSerDeserNull<CFByteArray>(CFByteArray::fromHex("310032"), "c103 310032"));
        QVERIFY(checkSerDeserNull<CFByteArray>(CFByteArray::fromHex("313200"), "c103 313200"));

        QVERIFY(checkSerDeserNull<CFString>(CFString(),     ""));
        QVERIFY(checkSerDeserNull<CFString>(CFString(""),   "c100"));
        QVERIFY(checkSerDeserNull<CFString>(CFString("X"),  "c10158"));
        QVERIFY(checkSerDeserNull<CFString>(CFString("XY"), "c1025859"));
        QVERIFY(checkSerDeserNull<CFString>(CFString("XäÄöÖüÜßY"), "c1 10 58 c3a4 c384 c3b6 c396 c3bc c39c c39f 59"));

        QVERIFY(checkSerDeserNull<CFChar>(CFChar(),            ""));
        QVERIFY(checkSerDeserNull<CFChar>(CFChar{char32_t('A')}, "c10141"));
        QVERIFY(checkSerDeserNull<CFChar>(CFChar{char32_t(0xe4)}, "c10200e4"));
        QVERIFY(checkSerDeserNull<CFChar>(CFChar{char32_t(0xdf)}, "c10200df"));
    }

    void many()
    {
        {
            BERSerializer ser;
            ser << 17 << 18 << 0 << 20;
            const CFByteArray hex = CFByteArray::fromHex("C10111 C20112        C40114");
            QCOMPARE(ser.data(), hex);
            BERDeserializer deser(hex);
            int a, b, c, d; deser >> a >> b >> c >> d;
            QCOMPARE(a, 17);
            QCOMPARE(b, 18);
            QCOMPARE(c,  0);
            QCOMPARE(d, 20);
        }{
            BERSerializer ser;
            ser << 17 << Placeholder() << 19;
            const CFByteArray hex = CFByteArray::fromHex("C10111        C30113");
            QCOMPARE(ser.data(), hex);
            BERDeserializer deser(hex);
            int a, b; deser >> a >> Placeholder() >> b;
            QCOMPARE(a, 17);
            QCOMPARE(b, 19);
        }{
            const CFByteArray hex = CFByteArray::fromHex("C10111 C20112 C30113");
            BERDeserializer deser(hex);
            int a, c; deser >> a >> Placeholder() >> c;
            QCOMPARE(a, 17);
            QCOMPARE(c, 19);
        }
    }

    void bigTag()
    {
        QVERIFY(testBigTag(   30, "DE"));
        QVERIFY(testBigTag(   31, "DF1F"));
        QVERIFY(testBigTag(  127, "DF7f"));
        QVERIFY(testBigTag(  128, "DF8100"));
        QVERIFY(testBigTag(  129, "DF8101"));
        QVERIFY(testBigTag(  255, "DF817f"));
        QVERIFY(testBigTag(  256, "DF8200"));
        QVERIFY(testBigTag(16383, "DFff7f"));
        QVERIFY(testBigTag(16384, "DF818000"));
    }

    void object()
    {
        Test2 t2;
        t2.t1.a = 0x42;
        t2.t1.b = 0x23;
        t2.a = 0x43;
        QVERIFY(checkSerDeser<Test2>(t2, "e1 0b e1 06 c1 01 42 c2 01 23 c2 01 43"));
        t2.t1.a = 0;
        t2.t1.b = 0;
        QVERIFY(checkSerDeser<Test2>(t2, "E1 03 C2 01 43"));
        t2.a = 0;
        QVERIFY(checkSerDeser<Test2>(t2, ""));
        t2.t1.a = 0x42;
        QVERIFY(checkSerDeser<Test2>(t2, "e1 05 e1 03 c1 01 42"));
    }

    void lists()
    {
        QVERIFY(checkSerDeser<CFList<cfuint8>>(CFList<cfuint8>(), ""));
        QVERIFY(checkSerDeser<CFList<cfuint8>>(CFList<cfuint8>{0x42}, "e103c00142"));
        QVERIFY(checkSerDeser<CFList<cfuint8>>(CFList<cfuint8>{0x42, 0x43}, "e106c00142c00143"));
        QVERIFY(checkSerDeser<CFStringList>(CFStringList{"XY", CFString(), "", "A"},
            "e10c c0025859 c08100 c000 c00141"));

        BERDeserializer ser(CFByteArray::fromHex("e105 c08100 c000"));
        CFStringList list; ser >> list;
        QVERIFY(list[0].isNull());
        QVERIFY(!list[1].isNull());
        QVERIFY(list[1].isEmpty());
    }

    void maps()
    {
        typedef CFMap<CFString, int> Map;
        Map map;
        QVERIFY(checkSerDeser<Map>(map, ""));
        map["xy"] = 4;
        QVERIFY(checkSerDeser<Map>(map, "e107 c0027879 c00104"));
        map["xyz"] = 7;
        QVERIFY(checkSerDeser<Map>(map, "e10f c0027879 c00104 c00378797a c00107"));
    }
};

ADD_TEST(SerializeBER_Test)
