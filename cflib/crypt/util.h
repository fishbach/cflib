/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace crypt {

QByteArray random(uint size);
inline QByteArray randomId() { return random(20).toHex(); }
quint32 randomUInt32();
quint64 randomUInt64();
QByteArray memorableRandom();

QByteArray hashPassword(const QString & password);
bool checkPassword(const QString & password, const QByteArray & hash);

QByteArray sha1(const QByteArray & data);
QByteArray sha256(const QByteArray & data);

// PKCS#8 PEM encoded
QByteArray rsaCreateKey(uint bits);
bool rsaCheckKey(const QByteArray & privateKey);
void rsaPublicModulusExponent(const QByteArray & privateKey, QByteArray & modulus, QByteArray & publicExponent);
// PKCS#1 v1.5 SHA-256
QByteArray rsaSign(const QByteArray & privateKey, const QByteArray & msg);

// DER encoded
QByteArray x509CreateCertReq(const QByteArray & privateKey, const QList<QByteArray> subjectAltNames);

QByteArray der2pem(const QByteArray & der, const QByteArray & label);

}}    // namespace
