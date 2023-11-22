/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
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

QString writeStr(const QByteArray & msg)
{
    uint specialCount = 0;
    QByteArray rv;
    const char * start = msg.constData();
    const char * p = start;
    for (int i = 0 ; i < msg.length() ; ++i) {
        const quint8 c = (quint8)*p;
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
    return 100 * specialCount / msg.size() > 10 ? QString() : rv;
}

QString showValue(const quint8 * data, int len)
{
    if (len == 0) return "null";

    QString rv;

    QString str = writeStr(QByteArray::fromRawData((const char *)data, len));
    if (!str.isNull()) rv += '"' % str % "\" ";

    if (len == 9 && *data == 0) {
        quint64 val;
        impl::deserializeBERInt(val, data, len);
        rv += '(' % QString::number(val) % ") ";
    } else if (len < 9) {
        qint64 val;
        impl::deserializeBERInt(val, data, len);
        rv += '(' % QString::number(val);
        if (len == 4) {
            rv += " / ";
            rv += QString::number(*((const float *)data));
        } else if (len == 8) {
            rv += " / ";
            rv += QString::number(*((const double *)data));
        } else if (val >= 946681200000 && val < 4102441200000) {
            rv += " / ";
            rv += QDateTime::fromMSecsSinceEpoch(val, Qt::UTC).toString(Qt::ISODateWithMs);
        }
        rv += ") ";
    }

    rv += "0x" % QByteArray::fromRawData((const char *)data, len).toHex().toUpper();

    return rv;
}

QString printAsn1(const quint8 * data, int len, int indent)
{
    QString rv;

    forever {
        quint64 tagNo = 0;
        int tagLen = 0;
        int lengthSize = 0;
        const qint32 valueLen = getTLVLength(QByteArray::fromRawData((const char *)data, len), tagNo, tagLen, lengthSize);
        if (valueLen == -1) {
            rv << "not enough data available\n";
            return rv;
        }
        if (valueLen == -2) {
            rv << "undefined length found\n";
            return rv;
        }
        if (valueLen == -3) {
            rv << "too big length found\n";
            return rv;
        }

        for (int i = 0 ; i < indent ; ++i) rv << "  ";
        rv << QString("%1").arg(tagNo, 2, 10, QChar('0'));

        if (*data & 0x20) {
            rv << ":\n";
            rv << printAsn1(data + tagLen + lengthSize, valueLen, indent + 1);
        } else {
            rv << ": " << showValue(data + tagLen + lengthSize, valueLen) << "\n";
        }

        const qint32 total = tagLen + lengthSize + valueLen;
        if (len <= total) return rv;

        data += total;
        len  -= total;
    }

    return rv;
}

}

QString printAsn1(const QByteArray & data)
{
    return printAsn1((const quint8 *)data.constData(), data.size(), 0);
}

}}
