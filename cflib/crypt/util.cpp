/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "util.h"

#include <cflib/crypt/impl/botanhelper.h>

USE_LOG(LogCat::Crypt)

namespace cflib::crypt {

ByteArray random(uint size)
{
    TRY {
        AutoSeeded_RNG rng;
        ByteArray retval(size, '\0');
        rng.randomize((byte *)retval.data(), size);
        return retval;
    } CATCH
    return ByteArray();
}

cfuint32 randomUInt32()
{
    TRY {
        AutoSeeded_RNG rng;
        cfuint32 retval = 0;
        rng.randomize((byte *)&retval, 4);
        return retval;
    } CATCH
    return 0;
}

cfuint64 randomUInt64()
{
    TRY {
        AutoSeeded_RNG rng;
        cfuint64 retval = 0;
        rng.randomize((byte *)&retval, 8);
        return retval;
    } CATCH
    return 0;
}

ByteArray memorableRandom(const int length)
{
    const char * vowels     = "aeiou";
    const char * consonants = "bcdfghjklmnpqrstvwxyz";
    const ByteArray rnd = random(length+2);
    if ((int)rnd.size() != length+2) return ByteArray();
    ByteArray rv(length+2, '\0');
    for (int i = 0 ; i < length ; ++i) rv[i] = (i % 2 == 0) ? consonants[(cfuint8)rnd[i] * 21 / 256] : vowels[(cfuint8)rnd[i] * 5 / 256];
    for (int i = length ; i < length+2 ; ++i) rv[i] = '0' + ((cfuint8)rnd[i] * 10 / 256);
    return rv;
}

ByteArray hashPassword(const String & password)
{
    TRY {
        AutoSeeded_RNG rng;
        std::string hash = generate_bcrypt(password.str(), rng);
        return ByteArray(hash.c_str(), (cfsize_t)hash.length());
    } CATCH
    return ByteArray();
}

bool checkPassword(const String & password, const ByteArray & hash)
{
    TRY {
        return check_bcrypt(password.str(), std::string(hash.constData(), hash.length()));
    } CATCH
    return false;
}

ByteArray sha1(const ByteArray & data)
{
    TRY {
        Pipe pipe(new Hash_Filter("SHA-1"));
        pipe.process_msg((const byte *)data.constData(), data.size());
        std::string hash = pipe.read_all_as_string();
        return ByteArray(hash.c_str(), (cfsize_t)hash.length());
    } CATCH
    return ByteArray();
}

ByteArray sha256(const ByteArray & data)
{
    TRY {
        Pipe pipe(new Hash_Filter("SHA-256"));
        pipe.process_msg((const byte *)data.constData(), data.size());
        std::string hash = pipe.read_all_as_string();
        return ByteArray(hash.c_str(), (cfsize_t)hash.length());
    } CATCH
    return ByteArray();
}

ByteArray rsaCreateKey(uint bits)
{
    TRY {
        AutoSeeded_RNG rng;
        const RSA_PrivateKey key(rng, bits);
        const std::string pem = PKCS8::PEM_encode(key);
        return ByteArray(pem.c_str(), (cfsize_t)pem.length());
    } CATCH
    return ByteArray();
}

bool rsaCheckKey(const ByteArray & privateKey)
{
    TRY {
        DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
        std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds));
        AutoSeeded_RNG rng;
        return pk && pk->check_key(rng, true);
    } CATCH
    return false;
}

void rsaPublicModulusExponent(const ByteArray & privateKey, ByteArray & modulus, ByteArray & publicExponent)
{
    TRY {
        DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
        std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds));
        if (pk) {
            const RSA_PublicKey * rsaKey = dynamic_cast<const RSA_PrivateKey *>(pk.get());
            std::vector<byte> bytes = rsaKey->get_n().serialize<std::vector<uint8_t>>();
            modulus = ByteArray((const char *)bytes.data(), (cfsize_t)bytes.size());
            bytes = rsaKey->get_e().serialize<std::vector<uint8_t>>();
            publicExponent = ByteArray((const char *)bytes.data(), (cfsize_t)bytes.size());
            return;
        }
    } CATCH
    modulus = ByteArray();
    publicExponent = ByteArray();
}

ByteArray rsaSign(const ByteArray & privateKey, const ByteArray & msg)
{
    TRY {
        DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
        std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds));
        if (pk) {
            AutoSeeded_RNG rng;
            PK_Signer signer(*pk, rng, "EMSA3(SHA-256)");
            std::vector<byte> bytes = signer.sign_message((const byte *)msg.constData(), msg.size(), rng);
            return ByteArray((const char *)bytes.data(), (cfsize_t)bytes.size());
        }
    } CATCH
    return ByteArray();
}

ByteArray x509CreateCertReq(const ByteArray & privateKey, const CFList<ByteArray> subjectAltNames)
{
    TRY {
        DataSource_Memory ds((const byte *)privateKey.constData(), privateKey.size());
        std::unique_ptr<Private_Key> pk(PKCS8::load_key(ds));
        if (!pk) return ByteArray();

        const size_t PKCS10_VERSION = 0;

        Extensions extensions;
        {
            AlternativeName subjectAN;
            for (const ByteArray & an : subjectAltNames) subjectAN.add_dns(std::string(an.constData(), an.size()));
            extensions.add(std::unique_ptr<Certificate_Extension>(new Cert_Extension::Subject_Alternative_Name(subjectAN)));
        }

        std::vector<uint8_t> extensionAttribute;
        DER_Encoder(extensionAttribute)
            .start_cons(ASN1_Type::Sequence, ASN1_Class::Universal)
            .encode(extensions)
            .end_cons();

        DER_Encoder der;
        der.start_cons(ASN1_Type::Sequence, ASN1_Class::Universal)
            .encode(PKCS10_VERSION)
            .encode(X509_DN())
            .raw_bytes(X509::BER_encode(*pk))
            .start_explicit(0)
            .encode(Attribute("PKCS9.ExtensionRequest", extensionAttribute))
            .end_explicit()
            .end_cons();

        AutoSeeded_RNG rng;
        std::unique_ptr<PK_Signer> signer(PKCS10_Request::choose_sig_format(*pk, rng, "SHA-256", ""));

        PKCS10_Request csr = PKCS10_Request(X509_Object::make_signed(*signer, rng, signer->algorithm_identifier(), der.get_contents()));

        std::vector<byte> bytes = csr.BER_encode();
        return ByteArray((const char *)bytes.data(), (cfsize_t)bytes.size());
    } CATCH
    return ByteArray();
}

ByteArray der2pem(const ByteArray & der, const ByteArray & label)
{
    TRY {
        const std::string pem = PEM_Code::encode(
            (const byte *)der.constData(), der.size(), std::string(label.constData(), label.size()));
        return ByteArray(pem.c_str(), (cfsize_t)pem.size());
    } CATCH
    return ByteArray();
}

} // namespace
