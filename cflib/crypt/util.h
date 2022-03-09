/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
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

}}	// namespace
