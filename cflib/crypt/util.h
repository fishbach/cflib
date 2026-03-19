/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::crypt {

ByteArray random(uint size);
inline ByteArray randomId() { return random(20).toHex(); }
cfuint32 randomUInt32();
cfuint64 randomUInt64();
ByteArray memorableRandom(const int length = 6);

ByteArray hashPassword(const String & password);
bool checkPassword(const String & password, const ByteArray & hash);

ByteArray sha1(const ByteArray & data);
ByteArray sha256(const ByteArray & data);

// PKCS#8 PEM encoded
ByteArray rsaCreateKey(uint bits);
bool rsaCheckKey(const ByteArray & privateKey);
void rsaPublicModulusExponent(const ByteArray & privateKey, ByteArray & modulus, ByteArray & publicExponent);
// PKCS#1 v1.5 SHA-256
ByteArray rsaSign(const ByteArray & privateKey, const ByteArray & msg);

// DER encoded
ByteArray x509CreateCertReq(const ByteArray & privateKey, const List<ByteArray> subjectAltNames);

ByteArray der2pem(const ByteArray & der, const ByteArray & label);

} // namespace
