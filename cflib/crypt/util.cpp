/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
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

#include "util.h"

#include <cflib/crypt/impl/botanhelper.h>

USE_LOG(LogCat::Crypt)

namespace cflib { namespace crypt {

QByteArray random(uint size)
{
	TRY {
		AutoSeeded_RNG rng;
		QByteArray retval(size, '\0');
		rng.randomize((byte *)retval.constData(), size);
		return retval;
	} CATCH
	return QByteArray();
}

quint32 randomUInt32()
{
	TRY {
		AutoSeeded_RNG rng;
		quint32 retval = 0;
		rng.randomize((byte *)&retval, 4);
		return retval;
	} CATCH
	return 0;
}

quint64 randomUInt64()
{
	TRY {
		AutoSeeded_RNG rng;
		quint64 retval = 0;
		rng.randomize((byte *)&retval, 8);
		return retval;
	} CATCH
	return 0;
}

QByteArray memorableRandom()
{
	const char * vowels     = "aeiou";
	const char * consonants = "bcdfghjklmnpqrstvwxyz";
	const QByteArray rnd = random(8);
	if (rnd.size() != 8) return QByteArray();
	QByteArray rv(8, '\0');
	for (int i = 0 ; i < 6 ; ++i) rv[i] = (i % 2 == 0) ? consonants[(uchar)rnd[i] * 21 / 256] : vowels[(uchar)rnd[i] * 5 / 256];
	for (int i = 6 ; i < 8 ; ++i) rv[i] = '0' + ((uchar)rnd[i] * 10 / 256);
	return rv;
}

QByteArray hashPassword(const QString & password)
{
	TRY {
		AutoSeeded_RNG rng;
		std::string hash = generate_bcrypt(password.toStdString(), rng);
		return QByteArray(hash.c_str(), (int)hash.length());
	} CATCH
	return QByteArray();
}

bool checkPassword(const QString & password, const QByteArray & hash)
{
	TRY {
		return check_bcrypt(password.toStdString(), std::string(hash.constData(), hash.length()));
	} CATCH
	return false;
}

QByteArray sha1(const QByteArray & data)
{
	TRY {
		Pipe pipe(new Hash_Filter("SHA-1"));
		pipe.process_msg((const byte *)data.constData(), data.size());
		std::string hash = pipe.read_all_as_string();
		return QByteArray(hash.c_str(), (int)hash.length());
	} CATCH
	return QByteArray();
}

QByteArray sha256(const QByteArray & data)
{
	TRY {
		Pipe pipe(new Hash_Filter("SHA-256"));
		pipe.process_msg((const byte *)data.constData(), data.size());
		std::string hash = pipe.read_all_as_string();
		return QByteArray(hash.c_str(), (int)hash.length());
	} CATCH
	return QByteArray();
}

QByteArray rsaCreateKey(uint bits)
{
	TRY {
		AutoSeeded_RNG rng;
		const RSA_PrivateKey key(rng, bits);
		const std::string pem = PKCS8::PEM_encode(key);
		return QByteArray(pem.c_str(), pem.length());
	} CATCH
	return QByteArray();
}

bool rsaCheckKey(const QByteArray & privateKey)
{
	TRY {
		DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
		AutoSeeded_RNG rng;
		std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds, rng));
		return pk && pk->check_key(rng, true);
	} CATCH
	return false;
}

void rsaPublicModulusExponent(const QByteArray & privateKey, QByteArray & modulus, QByteArray & publicExponent)
{
	TRY {
		DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
		AutoSeeded_RNG rng;
		std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds, rng));
		if (pk) {
			const IF_Scheme_PublicKey * rsaKey = dynamic_cast<const RSA_PrivateKey *>(pk.get());
			std::vector<byte> bytes = BigInt::encode(rsaKey->get_n());
			modulus = QByteArray((const char *)bytes.data(), bytes.size());
			bytes = BigInt::encode(rsaKey->get_e());
			publicExponent = QByteArray((const char *)bytes.data(), bytes.size());
			return;
		}
	} CATCH
	modulus = QByteArray();
	publicExponent = QByteArray();
}

QByteArray rsaSign(const QByteArray & privateKey, const QByteArray & msg)
{
	TRY {
		DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
		AutoSeeded_RNG rng;
		std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds, rng));
		if (pk) {
			PK_Signer signer(*pk, "EMSA3(SHA-256)");
			std::vector<byte> bytes = signer.sign_message((const byte *)msg.constData(), msg.size(), rng);
			return QByteArray((const char *)bytes.data(), bytes.size());
		}
	} CATCH
	return QByteArray();
}

QByteArray x509CreateCertReq(const QByteArray & privateKey,
	const QByteArray & commonName, const QByteArray & country,
	const QList<QByteArray> subjectAltNames)
{
	TRY {
		X509_Cert_Options opts;
		opts.common_name = commonName.toStdString();
		opts.country = country.toUpper().toStdString();
		AutoSeeded_RNG rng;
		DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
		std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds, rng));
		if (pk) {
			PKCS10_Request request = X509::create_cert_req(opts, *pk, "SHA-256", rng);
			std::vector<byte> bytes = request.BER_encode();
			return QByteArray((const char *)bytes.data(), bytes.size());
		}
	} catch (std::exception & e) {
		QTextStream(stderr) << "Botan exception: " << e.what() << endl;
	} catch (...) { \
		QTextStream(stderr) << "unknown Botan exception" << endl;
	}

	return QByteArray();
}

}}	// namespace
