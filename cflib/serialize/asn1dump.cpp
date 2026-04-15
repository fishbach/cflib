/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "asn1dump.h"

#include <cflib/serialize/util.h>

#include <format>
#include <cflib/util/hex.h>
#include <cflib/util/util.h>

namespace cflib::serialize {

namespace {

String writeStr(const ByteArray & msg)
{
    uint specialCount = 0;
    ByteArray rv;
    const char * start = msg.constData();
    const char * p = start;
    for (size_t i = 0 ; i < msg.length() ; ++i) {
        const uint8 c = (uint8)*p;
        if (c < 0x20 || c > 0x7E) {
            if (p > start) rv.append(start, p - start);
            ++p; start = p;

            rv += '<';
            rv += util::toHex(c >> 4);
            rv += util::toHex(c & 0xF);
            rv += '>';
            ++specialCount;
        } else ++p;
    }
    if (p > start) rv.append(start, p - start);
    return 100 * specialCount / msg.size() > 10 ? String() : String(rv.constData());
}

String showValue(const uint8 * data, int len)
{
    if (len == 0) return "null";

    String rv;

    String str = writeStr(ByteArray::fromRawData((const char *)data, len));
    if (!str.isNull()) {
        rv += '"';
        rv += str;
        rv += "\" ";
    }

    if (len == 9 && *data == 0) {
        uint64 val;
        impl::deserializeBERInt(val, data, len);
        rv += '(';
        rv += String::number(val);
        rv += ") ";
    } else if (len < 9) {
        int64 val;
        impl::deserializeBERInt(val, data, len);
        rv += '(';
        rv += String::number(val);
        if (len == 4) {
            rv += " / ";
            float f;
            memcpy(&f, data, sizeof(float));
            rv += String::number((double)f);
        } else if (len == 8) {
            rv += " / ";
            double d;
            memcpy(&d, data, sizeof(double));
            rv += String::number(d);
        } else if (val >= 946681200000 && val < 4102441200000) {
            rv += " / ";
            DateTime dt = DateTime::fromMSecsSinceEpoch(val);
            // Format as ISO date
            rv += std::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}.{:03d}Z",
                dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), dt.msec()).c_str();
        }
        rv += ") ";
    }

    rv += "0x";
    rv += String(ByteArray::fromRawData((const char *)data, len).toHex().constData()).toUpper();

    return rv;
}

String printAsn1(const uint8 * data, int len, int indent)
{
    String rv;

    while (true) {
        uint64 tagNo = 0;
        int tagLen = 0;
        int lengthSize = 0;
        const int32 valueLen = getTLVLength(ByteArray::fromRawData((const char *)data, len), tagNo, tagLen, lengthSize);
        if (valueLen == -1) {
            rv += "not enough data available\n";
            return rv;
        }
        if (valueLen == -2) {
            rv += "undefined length found\n";
            return rv;
        }
        if (valueLen == -3) {
            rv += "too big length found\n";
            return rv;
        }

        for (int i = 0 ; i < indent ; ++i) rv += "  ";
        // Format tag number with leading zero
        rv += std::format("{:02}", (unsigned long long)tagNo).c_str();

        if (*data & 0x20) {
            rv += ":\n";
            rv += printAsn1(data + tagLen + lengthSize, valueLen, indent + 1);
        } else {
            rv += ": ";
            rv += showValue(data + tagLen + lengthSize, valueLen);
            rv += "\n";
        }

        const int32 total = tagLen + lengthSize + valueLen;
        if (len <= total) return rv;

        data += total;
        len  -= total;
    }

    return rv;
}

}

String printAsn1(const ByteArray & data)
{
    return printAsn1((const uint8 *)data.constData(), data.size(), 0);
}

}