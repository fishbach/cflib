/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "util.h"

#include <cflib/util/endian.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Http)

namespace cflib::net::impl {

ByteArray createHttpRequest(const ByteArray & method, const Url & url,
    const ByteArray & postData, const ByteArrayList & headers, bool keepAlive)
{
    // headline
    ByteArray rv = method;
    rv << ' ' << (url.path().isEmpty() ? "/" : url.path().toUtf8());
    if (url.hasQuery()) rv << "?" << url.query().toUtf8();
    rv << " HTTP/1.1\r\n";

    // host
    rv << "Host: " << url.host().toUtf8();
    if (url.port() != -1) rv << ":" << ByteArray::fromInt(url.port());
    rv << "\r\n";

    // login / password
    if (!url.userInfo().isEmpty()) {
        rv << "Authorization: Basic " << url.userInfo().toUtf8().toBase64() << "\r\n";
    }

    // keep alive
    if (keepAlive) rv << "Connection: keep-alive\r\n";

    // custom headers
    for (const ByteArray & header : headers) {
        const ByteArray h = header.trimmed();
        if (!h.isEmpty()) rv << h << "\r\n";
    }

    // content and length
    if (postData.isNull()) {
        rv << "\r\n";
    } else {
        rv
            << "Content-Length: " << ByteArray::fromInt(postData.size()) << "\r\n"
            << "\r\n"
            << postData;
    }

    return rv;
}

namespace {

// see: https://datatracker.ietf.org/doc/html/rfc2616#section-4
bool parseHeader(const ByteArray & header, MultiMap<ByteArray, ByteArray> & fields,
    std::function<bool (const ByteArrayList & line)> checkHeader)
{
    bool isFirst = true;
    int start = 0;
    int end = header.indexOf("\r\n", 0);
    if (end < 0) end = header.size();
    while (start < end) {
        const ByteArray line = header.mid(start, end - start);
        start = end + 2;
        end = header.indexOf("\r\n", start);
        if (end < 0) end = header.size();

        if (isFirst) {
            isFirst = false;
            ByteArrayList parts = line.split(' ');
            if (parts.size() < 3) {
                logWarn("unknown request %1", line);
                return false;
            }
            for (int i = 3 ; i < (int)parts.size() ; ++i) parts[2] << ' ' << parts[i];
            if (!checkHeader(parts)) return false;
            continue;
        }

        const int pos = line.indexOf(':');
        if (pos == -1) {
            logWarn("funny line in header: %1", line);
            return false;
        }

        fields.insert(line.left(pos).toLower(), line.mid(pos + (line[pos + 1] == ' ' ? 2 : 1)));
    }

    // returns true only if the header is correct and not empty (just containing \r\n)
    return !isFirst;
}

}

bool parseRequestHeader(const ByteArray & header,
    ByteArray & method, ByteArray & uri, MultiMap<ByteArray, ByteArray> & fields)
{
    return parseHeader(header, fields, [&](const ByteArrayList & parts) {
        if (!parts[2].startsWith("HTTP/1.")) {
            logWarn("unknown protocol %1", parts[2]);
            return false;
        }

        method = parts[0];
        if (method.isEmpty()) {
            logWarn("no method");
            return false;
        }

        uri = parts[1];
        if (uri.isEmpty()) {
            logWarn("no URI");
            return false;
        }

        return true;
    });
}

bool parseResponseHeader(const ByteArray & header,
    int & status, ByteArray & statusText, MultiMap<ByteArray, ByteArray> & fields)
{
    return parseHeader(header, fields, [&](const ByteArrayList & parts) {
        if (!parts[0].startsWith("HTTP/1.")) {
            logWarn("unknown protocol %1", parts[0]);
            return false;
        }

        bool ok;
        status = parts[1].toInt(&ok);
        if (!ok) {
            logWarn("broken status: %1", parts[1]);
            return false;
        }

        statusText = parts[2];

        return true;
    });
}

namespace ws {

bool readLength(const ByteArray & frame,
    bool & fin, bool & rsv1, uint8 & opcode,
    bool & mask, uint64 & len, uint & lengthEnd)
{
    const int frameSize = frame.size();
    if (frameSize < 2) return false;

    const uint8 * const data = (const uint8 *)frame.constCharPtr();
    const uint8 d0 = data[0];
    const uint8 d1 = data[1];

    fin    = d0 & impl::ws::Fin;
    rsv1   = d0 & impl::ws::Rsv1;
    opcode = d0 & impl::ws::OpcodeBits;
    mask   = d1 & impl::ws::Mask;

    len = d1 & ~impl::ws::Mask;
    if (len < 126) {
        lengthEnd = 2;
    } else if (len == 126) {
        if (frameSize < 4) return false;
        len = util::readBE16(data + 2);
        lengthEnd = 4;
    } else {
        if (frameSize < 10) return false;
        len = util::readBE64(data + 2);
        lengthEnd = 10;
    }

    if (mask) len += 4;

    return (uint64)frameSize >= lengthEnd + len;
}

void writeLength(ByteArray & frame,
    bool fin, bool rsv1, uint8 opcode,
    bool mask, uint64 keyAndPayloadSize, uint64 lengthSize)
{
    /* If <lengthSize> is zero (it is per default) then the <frame> is interpreted
    * as including the mask key and the payload, to which the "length field" is prepended to.
    * For optimization purposes, if lengthSize is greater than 0 then this method
    * interpretes the <frame> as a frame including the payload starting at
    * <lengthSize> bytes + 4 bytes (length of mask key) after the start of the frame.
    * The length field is then written into the start of the frame.
    *   frame:  [ length field | mask key (4 bytes) | payload ]
    */
    uint64 payloadSize = mask ? keyAndPayloadSize - 4 : keyAndPayloadSize;

    uint8 d0 = opcode;
    if (fin ) d0 |= impl::ws::Fin;
    if (rsv1) d0 |= impl::ws::Rsv1;

    if (lengthSize == 0) {
        frame += d0;

        if (payloadSize  < 126) {
            frame += (uint8)payloadSize | (mask ? Mask : 0);
        } else if (payloadSize < 0x10000) {
            frame += 126 | (mask ? Mask : 0);
            uint8 bytes[2];
            util::writeBE16(bytes, payloadSize);
            frame.append((const char *)bytes, 2);
        } else {
            frame += 127 | (mask ? Mask : 0);
            uint8 bytes[8];
            util::writeBE64(bytes, payloadSize);
            frame.append((const char *)bytes, 8);
        }
    } else {
        frame[0] = d0;
        switch(lengthSize) {
        case 2:    // payloadLength < 126
            frame[1] = (uint8)payloadSize | (mask ? Mask : 0);
            break;
        case 4:    // payloadLength < 0x10000
            frame[1] = 126 | (mask ? Mask : 0);
            util::writeBE16((uint8 *)(frame.constData() + 2), payloadSize);
            break;
        case 10:   // payloadLength >= 0x10000
            frame[1] = 127 | (mask ? Mask : 0);
            util::writeBE16((uint8 *)(frame.constData() + 2), payloadSize);
            break;
        default:   // Invalid
            logWarn("Invalid lengthSize %1 of frame with payload size %2", lengthSize, payloadSize);
        }
    }

}

void maskPayload(ByteArray & frame, uint32 key, uint32 offsetFront)
{
    /* If <offsetFront> is zero (it is per default) then the <frame> is interpreted
     * as only the payload, to which the <key> is prepended to.
     * For optimization purposes, if offsetFront is greater than 0 then this method
     * interpretes the <frame> as a frame including the payload starting at
     * <offsetFront> + 4 bytes after the start of the frame.
     * The <key> value is then written to <offsetFront> bytes after start of the frame.
     *   frame:  [ offsetFront | key (4 bytes) | payload ]
     */

    // endianness is not relevant here
    const uint8 * keyBytes = (const uint8 *)&key;
    uint8 * data = (uint8 *)frame.constData();
    if (offsetFront > 0) {
        for (int i = 0 ; i < (int)(frame.size() - offsetFront - 4) ; ++i) data[offsetFront + 4 + i] ^= keyBytes[i % 4];
        for (int i = 0 ; i < 4 ; ++i) data[offsetFront + i] = keyBytes[i];
    } else {
        for (int i = 0 ; i < (int)frame.size() ; ++i) data[i] ^= keyBytes[i % 4];
        frame.prepend((const char *)keyBytes, 4);
    }
}

void unmaskPayload(ByteArray & payload)
{
    // endianness is not relevant here
    uint8 * keyBytes = (uint8 *)payload.constData();
    uint8 * data = keyBytes + 4;
    int len = payload.size() - 4;
    for (int i = 0 ; i < len ; ++i) data[i] ^= keyBytes[i % 4];
    payload.remove(0, 4);
}

}

} // namespace
