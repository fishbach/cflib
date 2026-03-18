/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib { namespace crypt {

CFByteArray random(uint size);
inline CFByteArray randomId() { return random(20).toHex(); }
cfuint32 randomUInt32();
cfuint64 randomUInt64();
CFByteArray memorableRandom(const int length = 6);

CFByteArray hashPassword(const CFString & password);
bool checkPassword(const CFString & password, const CFByteArray & hash);

CFByteArray sha1(const CFByteArray & data);
CFByteArray sha256(const CFByteArray & data);

// PKCS#8 PEM encoded
CFByteArray rsaCreateKey(uint bits);
bool rsaCheckKey(const CFByteArray & privateKey);
void rsaPublicModulusExponent(const CFByteArray & privateKey, CFByteArray & modulus, CFByteArray & publicExponent);
// PKCS#1 v1.5 SHA-256
CFByteArray rsaSign(const CFByteArray & privateKey, const CFByteArray & msg);

// DER encoded
CFByteArray x509CreateCertReq(const CFByteArray & privateKey, const CFList<CFByteArray> subjectAltNames);

CFByteArray der2pem(const CFByteArray & der, const CFByteArray & label);

}}    // namespace
