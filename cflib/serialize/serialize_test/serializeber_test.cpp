/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/serialize_test/test.h>
#include <cflib/util/test.h>

#include <format>
#include <iostream>

using namespace cflib::serialize;

namespace {

template<typename T>
bool checkSer(T val, const char * hex)
{
    BERSerializer ser;
    ser << val;
    const ByteArray expected = ByteArray::fromHex(hex);
    if (ser.data() != expected) {
        std::cout << std::format("serialized hex differs:\nis       : {}\nexpected : {}\n",
            ser.data().toHex().data(), expected.toHex().data());
        return false;
    }
    return true;
}

template<typename T>
bool checkDeser(T val, const char * hex)
{
    const ByteArray expected = ByteArray::fromHex(hex);
    BERDeserializer deser(expected);
    T test; deser >> test;
    if (test != val) {
        std::cout << "deserialized values differ\n";
        return false;
    }
    return true;
}

template<typename T>
bool checkDeserNull(T val, const char * hex)
{
    const ByteArray expected = ByteArray::fromHex(hex);
    BERDeserializer deser(expected);
    T test; deser >> test;
    if (test.isNull() != val.isNull()) {
        std::cout << std::format("deserialized isNull differs: is={} expected={}\n",
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
    const ByteArray expected = ByteArray::fromHex(hex) + ByteArray::fromHex("0142");
    if (ser.data() != expected) {
        std::cout << std::format("serialized hex differs:\nis       : {}\nexpected : {}\n",
            ser.data().toHex().data(), expected.toHex().data());
        retval = false;
    }
    BERDeserializer deser(expected);
    for (int i = 0 ; i < tagNr - 1 ; ++i) deser >> Placeholder();
    int test; deser >> test;
    if (test != 0x42) {
        std::cout << std::format("deserialized values differ: is={} expected={}\n", test, 0x42);
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
        TVERIFY(checkSerDeser<bool      >( false, ""          ));
        TVERIFY(checkSerDeser<bool      >(  true, "c10101"    ));
        TVERIFY(checkSerDeser<cfuint8   >(     0, ""          ));
        TVERIFY(checkSerDeser<cfuint8   >(     1, "c10101"    ));
        TVERIFY(checkSerDeser<cfuint8   >(   127, "c1017f"    ));
        TVERIFY(checkSerDeser<cfuint8   >(   255, "c10200ff"  ));
        TVERIFY(checkSerDeser<cfint8    >(     0, ""          ));
        TVERIFY(checkSerDeser<cfint8    >(     1, "c10101"    ));
        TVERIFY(checkSerDeser<cfint8    >(   127, "c1017f"    ));
        TVERIFY(checkSerDeser<cfint8    >(    -1, "c101ff"    ));
        TVERIFY(checkSerDeser<cfint8    >(    -2, "c101fe"    ));
        TVERIFY(checkSerDeser<cfint8    >(  -128, "c10180"    ));
        TVERIFY(checkSerDeser<cfuint16  >(     0, ""          ));
        TVERIFY(checkSerDeser<cfuint16  >(     1, "c10101"    ));
        TVERIFY(checkSerDeser<cfuint16  >( 32767, "c1027fff"  ));
        TVERIFY(checkSerDeser<cfuint16  >( 65535, "c10300ffff"));
        TVERIFY(checkSerDeser<cfint16   >(     0, ""          ));
        TVERIFY(checkSerDeser<cfint16   >(     1, "c10101"    ));
        TVERIFY(checkSerDeser<cfint16   >(   127, "c1017f"    ));
        TVERIFY(checkSerDeser<cfint16   >(   128, "c1020080"  ));
        TVERIFY(checkSerDeser<cfint16   >( 32767, "c1027fff"  ));
        TVERIFY(checkSerDeser<cfint16   >(    -1, "c101ff"    ));
        TVERIFY(checkSerDeser<cfint16   >(    -2, "c101fe"    ));
        TVERIFY(checkSerDeser<cfint16   >(  -128, "c10180"    ));
        TVERIFY(checkSerDeser<cfint16   >(  -129, "c102ff7f"  ));
        TVERIFY(checkSerDeser<cfint16   >(-32767, "c1028001"  ));
        TVERIFY(checkSerDeser<cfint16   >(-32768, "c1028000"  ));
        TVERIFY(checkSerDeser<cfuint64  >(     0, ""          ));
        TVERIFY(checkSerDeser<cfuint64  >(     1, "c10101"    ));
        TVERIFY(checkSerDeser<cfuint64  >(   128, "c1020080"  ));
        TVERIFY(checkSerDeser<cfint64   >(    -1, "c101ff"    ));
        TVERIFY(checkSerDeser<cfint64   >(    -2, "c101fe"    ));
        TVERIFY(checkSerDeser<cfint64   >(  -128, "c10180"    ));
        TVERIFY(checkSerDeser<cfint64   >(  -129, "c102ff7f"  ));

        TVERIFY(checkSerDeser<cfuint64>((cfuint64)UINT64_C(0x7fffffffffffffff), "c1087fffffffffffffff"));
        TVERIFY(checkSerDeser<cfuint64>((cfuint64)UINT64_C(0x8000000000000000), "c109008000000000000000"));
        TVERIFY(checkSerDeser<cfuint64>((cfuint64)UINT64_C(0xfffffffffffffffe), "c10900fffffffffffffffe"));
        TVERIFY(checkSerDeser<cfuint64>((cfuint64)UINT64_C(0xffffffffffffffff), "c10900ffffffffffffffff"));

        TVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C(   36028797018963967),   "c1077fffffffffffff"));
        TVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C(   36028797018963968),   "c1080080000000000000"));
        TVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C( 9223372036854775807),   "c1087fffffffffffffff"));
        TVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C(-9223372036854775807),   "c1088000000000000001"));
        TVERIFY(checkSerDeser<cfint64>((cfint64)INT64_C(-9223372036854775807)-1, "c1088000000000000000"));
    }

    void nullInList()
    {
        TVERIFY(checkSerDeser<CFList<bool      >>(CFList<bool      >{false}, "e103c08100"));
        TVERIFY(checkSerDeser<CFList<cfuint8   >>(CFList<cfuint8   >{0},     "e103c08100"));
        TVERIFY(checkSerDeser<CFList<cfint8    >>(CFList<cfint8    >{0},     "e103c08100"));
        TVERIFY(checkSerDeser<CFList<cfuint16  >>(CFList<cfuint16  >{0},     "e103c08100"));
        TVERIFY(checkSerDeser<CFList<cfuint64  >>(CFList<cfuint64  >{0},     "e103c08100"));

        TVERIFY(checkSer<CFList<const char *>>(CFList<const char *>{nullptr},          "e103c08100"));
        TVERIFY(checkSerDeser<CFList<ByteArray>>(CFList<ByteArray>{ByteArray()}, "e103c08100"));
        TVERIFY(checkSerDeser<CFList<String   >>(CFList<String   >{String()},    "e103c08100"));
    }

    void string()
    {
        TVERIFY(checkSer<const char *>(nullptr, ""));
        TVERIFY(checkSer<const char *>("",      "c100"));
        TVERIFY(checkSer<const char *>("X",     "c10158"));
        TVERIFY(checkSer<const char *>("XY",    "c1025859"));

        TVERIFY(checkSerDeserNull<ByteArray>(ByteArray(),     ""));
        TVERIFY(checkSerDeserNull<ByteArray>(ByteArray(""),   "c100"));
        TVERIFY(checkSerDeserNull<ByteArray>(ByteArray("X"),  "c10158"));
        TVERIFY(checkSerDeserNull<ByteArray>(ByteArray("XY"), "c1025859"));
        TVERIFY(checkSerDeserNull<ByteArray>(ByteArray::fromHex("003132"), "c103 003132"));
        TVERIFY(checkSerDeserNull<ByteArray>(ByteArray::fromHex("310032"), "c103 310032"));
        TVERIFY(checkSerDeserNull<ByteArray>(ByteArray::fromHex("313200"), "c103 313200"));

        TVERIFY(checkSerDeserNull<String>(String(),     ""));
        TVERIFY(checkSerDeserNull<String>(String(""),   "c100"));
        TVERIFY(checkSerDeserNull<String>(String("X"),  "c10158"));
        TVERIFY(checkSerDeserNull<String>(String("XY"), "c1025859"));
        TVERIFY(checkSerDeserNull<String>(String("XäÄöÖüÜßY"), "c1 10 58 c3a4 c384 c3b6 c396 c3bc c39c c39f 59"));
    }

    void many()
    {
        {
            BERSerializer ser;
            ser << 17 << 18 << 0 << 20;
            const ByteArray hex = ByteArray::fromHex("C10111 C20112        C40114");
            TCOMPARE(ser.data(), hex);
            BERDeserializer deser(hex);
            int a, b, c, d; deser >> a >> b >> c >> d;
            TCOMPARE(a, 17);
            TCOMPARE(b, 18);
            TCOMPARE(c,  0);
            TCOMPARE(d, 20);
        }{
            BERSerializer ser;
            ser << 17 << Placeholder() << 19;
            const ByteArray hex = ByteArray::fromHex("C10111        C30113");
            TCOMPARE(ser.data(), hex);
            BERDeserializer deser(hex);
            int a, b; deser >> a >> Placeholder() >> b;
            TCOMPARE(a, 17);
            TCOMPARE(b, 19);
        }{
            const ByteArray hex = ByteArray::fromHex("C10111 C20112 C30113");
            BERDeserializer deser(hex);
            int a, c; deser >> a >> Placeholder() >> c;
            TCOMPARE(a, 17);
            TCOMPARE(c, 19);
        }
    }

    void bigTag()
    {
        TVERIFY(testBigTag(   30, "DE"));
        TVERIFY(testBigTag(   31, "DF1F"));
        TVERIFY(testBigTag(  127, "DF7f"));
        TVERIFY(testBigTag(  128, "DF8100"));
        TVERIFY(testBigTag(  129, "DF8101"));
        TVERIFY(testBigTag(  255, "DF817f"));
        TVERIFY(testBigTag(  256, "DF8200"));
        TVERIFY(testBigTag(16383, "DFff7f"));
        TVERIFY(testBigTag(16384, "DF818000"));
    }

    void object()
    {
        Test2 t2;
        t2.t1.a = 0x42;
        t2.t1.b = 0x23;
        t2.a = 0x43;
        TVERIFY(checkSerDeser<Test2>(t2, "e1 0b e1 06 c1 01 42 c2 01 23 c2 01 43"));
        t2.t1.a = 0;
        t2.t1.b = 0;
        TVERIFY(checkSerDeser<Test2>(t2, "E1 03 C2 01 43"));
        t2.a = 0;
        TVERIFY(checkSerDeser<Test2>(t2, ""));
        t2.t1.a = 0x42;
        TVERIFY(checkSerDeser<Test2>(t2, "e1 05 e1 03 c1 01 42"));
    }

    void lists()
    {
        TVERIFY(checkSerDeser<CFList<cfuint8>>(CFList<cfuint8>(), ""));
        TVERIFY(checkSerDeser<CFList<cfuint8>>(CFList<cfuint8>{0x42}, "e103c00142"));
        TVERIFY(checkSerDeser<CFList<cfuint8>>(CFList<cfuint8>{0x42, 0x43}, "e106c00142c00143"));
        TVERIFY(checkSerDeser<StringList>(StringList{"XY", String(), "", "A"},
            "e10c c0025859 c08100 c000 c00141"));

        BERDeserializer ser(ByteArray::fromHex("e105 c08100 c000"));
        StringList list; ser >> list;
        TVERIFY(list[0].isNull());
        TVERIFY(!list[1].isNull());
        TVERIFY(list[1].isEmpty());
    }

    void maps()
    {
        typedef CFMap<String, int> Map;
        Map map;
        TVERIFY(checkSerDeser<Map>(map, ""));
        map["xy"] = 4;
        TVERIFY(checkSerDeser<Map>(map, "e107 c0027879 c00104"));
        map["xyz"] = 7;
        TVERIFY(checkSerDeser<Map>(map, "e10f c0027879 c00104 c00378797a c00107"));
    }
};

ADD_TEST(SerializeBER_Test)
