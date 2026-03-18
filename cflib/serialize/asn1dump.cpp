/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "asn1dump.h"

#include <cflib/serialize/util.h>
#include <cflib/util/hex.h>
#include <cflib/util/util.h>

namespace cflib { namespace serialize {

namespace {

CFString writeStr(const CFByteArray & msg)
{
    uint specialCount = 0;
    CFByteArray rv;
    const char * start = msg.constData();
    const char * p = start;
    for (cfsize_t i = 0 ; i < msg.length() ; ++i) {
        const cfuint8 c = (cfuint8)*p;
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
    return 100 * specialCount / msg.size() > 10 ? CFString() : CFString(rv.constData());
}

CFString showValue(const cfuint8 * data, int len)
{
    if (len == 0) return "null";

    CFString rv;

    CFString str = writeStr(CFByteArray::fromRawData((const char *)data, len));
    if (!str.isNull()) {
        rv += '"';
        rv += str;
        rv += "\" ";
    }

    if (len == 9 && *data == 0) {
        cfuint64 val;
        impl::deserializeBERInt(val, data, len);
        rv += '(';
        rv += CFString::number(val);
        rv += ") ";
    } else if (len < 9) {
        cfint64 val;
        impl::deserializeBERInt(val, data, len);
        rv += '(';
        rv += CFString::number(val);
        if (len == 4) {
            rv += " / ";
            rv += CFString::number((double)*((const float *)data));
        } else if (len == 8) {
            rv += " / ";
            rv += CFString::number(*((const double *)data));
        } else if (val >= 946681200000 && val < 4102441200000) {
            rv += " / ";
            CFDateTime dt = CFDateTime::fromMSecsSinceEpoch(val);
            // Format as ISO date
            char buf[64];
            snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second(), dt.msec());
            rv += buf;
        }
        rv += ") ";
    }

    rv += "0x";
    rv += CFString(CFByteArray::fromRawData((const char *)data, len).toHex().constData()).toUpper();

    return rv;
}

CFString printAsn1(const cfuint8 * data, int len, int indent)
{
    CFString rv;

    while (true) {
        cfuint64 tagNo = 0;
        int tagLen = 0;
        int lengthSize = 0;
        const cfint32 valueLen = getTLVLength(CFByteArray::fromRawData((const char *)data, len), tagNo, tagLen, lengthSize);
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
        char tagBuf[16];
        snprintf(tagBuf, sizeof(tagBuf), "%02llu", (unsigned long long)tagNo);
        rv += tagBuf;

        if (*data & 0x20) {
            rv += ":\n";
            rv += printAsn1(data + tagLen + lengthSize, valueLen, indent + 1);
        } else {
            rv += ": ";
            rv += showValue(data + tagLen + lengthSize, valueLen);
            rv += "\n";
        }

        const cfint32 total = tagLen + lengthSize + valueLen;
        if (len <= total) return rv;

        data += total;
        len  -= total;
    }

    return rv;
}

}

CFString printAsn1(const CFByteArray & data)
{
    return printAsn1((const cfuint8 *)data.constData(), data.size(), 0);
}

}}
