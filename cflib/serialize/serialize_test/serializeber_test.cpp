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

TEST_SUITE("BER") {

TEST_CASE("BER: integer")
{
    REQUIRE(checkSerDeser<bool      >( false, ""          ));
    REQUIRE(checkSerDeser<bool      >(  true, "c10101"    ));
    REQUIRE(checkSerDeser<uint8   >(     0, ""          ));
    REQUIRE(checkSerDeser<uint8   >(     1, "c10101"    ));
    REQUIRE(checkSerDeser<uint8   >(   127, "c1017f"    ));
    REQUIRE(checkSerDeser<uint8   >(   255, "c10200ff"  ));
    REQUIRE(checkSerDeser<int8    >(     0, ""          ));
    REQUIRE(checkSerDeser<int8    >(     1, "c10101"    ));
    REQUIRE(checkSerDeser<int8    >(   127, "c1017f"    ));
    REQUIRE(checkSerDeser<int8    >(    -1, "c101ff"    ));
    REQUIRE(checkSerDeser<int8    >(    -2, "c101fe"    ));
    REQUIRE(checkSerDeser<int8    >(  -128, "c10180"    ));
    REQUIRE(checkSerDeser<uint16  >(     0, ""          ));
    REQUIRE(checkSerDeser<uint16  >(     1, "c10101"    ));
    REQUIRE(checkSerDeser<uint16  >( 32767, "c1027fff"  ));
    REQUIRE(checkSerDeser<uint16  >( 65535, "c10300ffff"));
    REQUIRE(checkSerDeser<int16   >(     0, ""          ));
    REQUIRE(checkSerDeser<int16   >(     1, "c10101"    ));
    REQUIRE(checkSerDeser<int16   >(   127, "c1017f"    ));
    REQUIRE(checkSerDeser<int16   >(   128, "c1020080"  ));
    REQUIRE(checkSerDeser<int16   >( 32767, "c1027fff"  ));
    REQUIRE(checkSerDeser<int16   >(    -1, "c101ff"    ));
    REQUIRE(checkSerDeser<int16   >(    -2, "c101fe"    ));
    REQUIRE(checkSerDeser<int16   >(  -128, "c10180"    ));
    REQUIRE(checkSerDeser<int16   >(  -129, "c102ff7f"  ));
    REQUIRE(checkSerDeser<int16   >(-32767, "c1028001"  ));
    REQUIRE(checkSerDeser<int16   >(-32768, "c1028000"  ));
    REQUIRE(checkSerDeser<uint64  >(     0, ""          ));
    REQUIRE(checkSerDeser<uint64  >(     1, "c10101"    ));
    REQUIRE(checkSerDeser<uint64  >(   128, "c1020080"  ));
    REQUIRE(checkSerDeser<int64   >(    -1, "c101ff"    ));
    REQUIRE(checkSerDeser<int64   >(    -2, "c101fe"    ));
    REQUIRE(checkSerDeser<int64   >(  -128, "c10180"    ));
    REQUIRE(checkSerDeser<int64   >(  -129, "c102ff7f"  ));

    REQUIRE(checkSerDeser<uint64>((uint64)UINT64_C(0x7fffffffffffffff), "c1087fffffffffffffff"));
    REQUIRE(checkSerDeser<uint64>((uint64)UINT64_C(0x8000000000000000), "c109008000000000000000"));
    REQUIRE(checkSerDeser<uint64>((uint64)UINT64_C(0xfffffffffffffffe), "c10900fffffffffffffffe"));
    REQUIRE(checkSerDeser<uint64>((uint64)UINT64_C(0xffffffffffffffff), "c10900ffffffffffffffff"));

    REQUIRE(checkSerDeser<int64>((int64)INT64_C(   36028797018963967),   "c1077fffffffffffff"));
    REQUIRE(checkSerDeser<int64>((int64)INT64_C(   36028797018963968),   "c1080080000000000000"));
    REQUIRE(checkSerDeser<int64>((int64)INT64_C( 9223372036854775807),   "c1087fffffffffffffff"));
    REQUIRE(checkSerDeser<int64>((int64)INT64_C(-9223372036854775807),   "c1088000000000000001"));
    REQUIRE(checkSerDeser<int64>((int64)INT64_C(-9223372036854775807)-1, "c1088000000000000000"));
}

TEST_CASE("BER: nullInList")
{
    REQUIRE(checkSerDeser<List<bool    >>(List<bool>() << false, "e103c08100"));
    REQUIRE(checkSerDeser<List<uint8   >>(List<uint8   >{0},     "e103c08100"));
    REQUIRE(checkSerDeser<List<int8    >>(List<int8    >{0},     "e103c08100"));
    REQUIRE(checkSerDeser<List<uint16  >>(List<uint16  >{0},     "e103c08100"));
    REQUIRE(checkSerDeser<List<uint64  >>(List<uint64  >{0},     "e103c08100"));

    REQUIRE(checkSer<List<const char *>>(List<const char *>{nullptr},          "e103c08100"));
    REQUIRE(checkSerDeser<List<ByteArray>>(List<ByteArray>{ByteArray()}, "e103c08100"));
    REQUIRE(checkSerDeser<List<String   >>(List<String   >{String()},    "e103c08100"));
}

TEST_CASE("BER: string")
{
    REQUIRE(checkSer<const char *>(nullptr, ""));
    REQUIRE(checkSer<const char *>("",      "c100"));
    REQUIRE(checkSer<const char *>("X",     "c10158"));
    REQUIRE(checkSer<const char *>("XY",    "c1025859"));

    REQUIRE(checkSerDeserNull<ByteArray>(ByteArray(),     ""));
    REQUIRE(checkSerDeserNull<ByteArray>(ByteArray(""),   "c100"));
    REQUIRE(checkSerDeserNull<ByteArray>(ByteArray("X"),  "c10158"));
    REQUIRE(checkSerDeserNull<ByteArray>(ByteArray("XY"), "c1025859"));
    REQUIRE(checkSerDeserNull<ByteArray>(ByteArray::fromHex("003132"), "c103 003132"));
    REQUIRE(checkSerDeserNull<ByteArray>(ByteArray::fromHex("310032"), "c103 310032"));
    REQUIRE(checkSerDeserNull<ByteArray>(ByteArray::fromHex("313200"), "c103 313200"));

    REQUIRE(checkSerDeserNull<String>(String(),     ""));
    REQUIRE(checkSerDeserNull<String>(String(""),   "c100"));
    REQUIRE(checkSerDeserNull<String>(String("X"),  "c10158"));
    REQUIRE(checkSerDeserNull<String>(String("XY"), "c1025859"));
    REQUIRE(checkSerDeserNull<String>(String("XäÄöÖüÜßY"), "c1 10 58 c3a4 c384 c3b6 c396 c3bc c39c c39f 59"));
}

TEST_CASE("BER: many")
{
    {
        BERSerializer ser;
        ser << 17 << 18 << 0 << 20;
        const ByteArray hex = ByteArray::fromHex("C10111 C20112        C40114");
        REQUIRE_EQ(ser.data(), hex);
        BERDeserializer deser(hex);
        int a, b, c, d; deser >> a >> b >> c >> d;
        REQUIRE_EQ(a, 17);
        REQUIRE_EQ(b, 18);
        REQUIRE_EQ(c,  0);
        REQUIRE_EQ(d, 20);
    }{
        BERSerializer ser;
        ser << 17 << Placeholder() << 19;
        const ByteArray hex = ByteArray::fromHex("C10111        C30113");
        REQUIRE_EQ(ser.data(), hex);
        BERDeserializer deser(hex);
        int a, b; deser >> a >> Placeholder() >> b;
        REQUIRE_EQ(a, 17);
        REQUIRE_EQ(b, 19);
    }{
        const ByteArray hex = ByteArray::fromHex("C10111 C20112 C30113");
        BERDeserializer deser(hex);
        int a, c; deser >> a >> Placeholder() >> c;
        REQUIRE_EQ(a, 17);
        REQUIRE_EQ(c, 19);
    }
}

TEST_CASE("BER: bigTag")
{
    REQUIRE(testBigTag(   30, "DE"));
    REQUIRE(testBigTag(   31, "DF1F"));
    REQUIRE(testBigTag(  127, "DF7f"));
    REQUIRE(testBigTag(  128, "DF8100"));
    REQUIRE(testBigTag(  129, "DF8101"));
    REQUIRE(testBigTag(  255, "DF817f"));
    REQUIRE(testBigTag(  256, "DF8200"));
    REQUIRE(testBigTag(16383, "DFff7f"));
    REQUIRE(testBigTag(16384, "DF818000"));
}

TEST_CASE("BER: object")
{
    Test2 t2;
    t2.t1.a = 0x42;
    t2.t1.b = 0x23;
    t2.a = 0x43;
    REQUIRE(checkSerDeser<Test2>(t2, "e1 0b e1 06 c1 01 42 c2 01 23 c2 01 43"));
    t2.t1.a = 0;
    t2.t1.b = 0;
    REQUIRE(checkSerDeser<Test2>(t2, "E1 03 C2 01 43"));
    t2.a = 0;
    REQUIRE(checkSerDeser<Test2>(t2, ""));
    t2.t1.a = 0x42;
    REQUIRE(checkSerDeser<Test2>(t2, "e1 05 e1 03 c1 01 42"));
}

TEST_CASE("BER: lists")
{
    REQUIRE(checkSerDeser<List<uint8>>(List<uint8>(), ""));
    REQUIRE(checkSerDeser<List<uint8>>(List<uint8>{0x42}, "e103c00142"));
    REQUIRE(checkSerDeser<List<uint8>>(List<uint8>{0x42, 0x43}, "e106c00142c00143"));
    REQUIRE(checkSerDeser<StringList>(StringList{"XY", String(), "", "A"},
        "e10c c0025859 c08100 c000 c00141"));

    BERDeserializer ser(ByteArray::fromHex("e105 c08100 c000"));
    StringList list; ser >> list;
    REQUIRE(list[0].isNull());
    REQUIRE(!list[1].isNull());
    REQUIRE(list[1].isEmpty());
}

TEST_CASE("BER: maps")
{
    typedef Map<String, int> Map;
    Map map;
    REQUIRE(checkSerDeser<Map>(map, ""));
    map["xy"] = 4;
    REQUIRE(checkSerDeser<Map>(map, "e107 c0027879 c00104"));
    map["xyz"] = 7;
    REQUIRE(checkSerDeser<Map>(map, "e10f c0027879 c00104 c00378797a c00107"));
}

}
