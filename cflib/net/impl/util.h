/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::net::impl {

ByteArray createHttpRequest(const ByteArray & method, const Url & url,
    const ByteArray & postData, const ByteArrayList & headers, bool keepAlive);

bool parseRequestHeader (const ByteArray & header,
    ByteArray & method, ByteArray & uri, MultiMap<ByteArray, ByteArray> & fields);
bool parseResponseHeader(const ByteArray & header,
    int & status, ByteArray & statusText, MultiMap<ByteArray, ByteArray> & fields);

// WebSocket (RFC 6455, RFC 7692)
namespace ws {

enum Opcode : uint8 {
    OpcodeBits        = 0x0F,

    ContinuationFrame = 0x00,
    TextFrame         = 0x01,
    BinaryFrame       = 0x02,
    ConnectionClose   = 0x08,
    Ping              = 0x09,
    Pong              = 0x0A,

    Fin               = 0x80,
    Rsv1              = 0x40,
    Rsv2              = 0x20,
    Rsv3              = 0x10
};

enum PayloadLen : uint8 {
    Mask = 0x80
};

bool readLength(const ByteArray & frame,
    bool & fin, bool & rsv1, uint8 & opcode,
    bool & mask, uint64 & len, uint & lengthEnd);
void writeLength(ByteArray & frame,
    bool fin, bool rsv1, uint8 opcode,
    bool mask, uint64 keyAndPayloadSize, uint64 lengthSize=0);
void maskPayload(ByteArray & frame, uint32 key, uint32 offsetFront=0);
void unmaskPayload(ByteArray & payload);

}

} // namespace
