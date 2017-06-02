/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#pragma once

#include <QtCore>

namespace {

/*

mkdir demoCA
cd demoCA
mkdir certs crl newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial
echo 01 > crlnumber
cd ..

openssl genrsa -out rootca_key.pem 2048
openssl req -new -sha256 -nodes -x509 -key rootca_key.pem -out rootca_crt.pem -days 3650 -subj '/CN=rootca'

openssl genrsa -out ca_key.pem 2048
openssl req -new -sha256 -key ca_key.pem -out ca_csr.pem -subj '/CN=ca'
openssl ca -md sha256 -keyfile rootca_key.pem -cert rootca_crt.pem -policy policy_anything -extensions v3_ca -in ca_csr.pem -out ca_crt.pem -days 3650 -batch
openssl ca -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -gencrl -out ca_crl.pem -crldays 3650

openssl genrsa -out server_key.pem 2048
openssl req -new -sha256 -key server_key.pem -out server_csr.pem -subj '/CN=127.0.0.1'
openssl ca -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -policy policy_anything -in server_csr.pem -out server_crt.pem -days 3650 -batch

openssl pkcs8 -v2 des3 -topk8 -in server_key.pem -out server_key_pkcs8.pem -passout pass:SuperSecure123

cat server_crt.pem server_key_pkcs8.pem ca_crt.pem ca_crl.pem rootca_crt.pem

*/

const QByteArray cert1(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDGDCCAgCgAwIBAgICEAEwDQYJKoZIhvcNAQELBQAwDTELMAkGA1UEAxMCY2Ew\r\n"
	"HhcNMTcwNjAyMDcxNTAxWhcNMjcwNTMxMDcxNTAxWjAUMRIwEAYDVQQDEwkxMjcu\r\n"
	"MC4wLjEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDmE5NOLKn7YViv\r\n"
	"QE2m4P2iCkQMPJAGBOQE+5QxDjEoXdR34PtbUrlpb0e6e2uzmEc+C42vzWsapzpX\r\n"
	"Zh0FILbbJEKUI+vVTyA5PK/9Qbl89g8bQMoUlc9o4IGNC/Dd2IDbQlsWaq5JvIss\r\n"
	"Ro9OUG1K8yYYEBH87uCK4G/0h+ivAQb07HxoxvgEVoiKvygSvjQwhK6Sndu2aY6N\r\n"
	"LwJhtC3lCLhROZfvZDZvmyGhsVYlqssf4f3gmjviOfvk7CiPeB/uO9XG5e9CUsmo\r\n"
	"X9sETN3HFotZh8fprgf/qQugmZeHJgQfDsmcFOqkekwp+y7Nv8VetQFbj+zYluwl\r\n"
	"+V0MSP97AgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJYIZIAYb4QgENBB8WHU9wZW5T\r\n"
	"U0wgR2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1UdDgQWBBTFdCGFu3agJs5oEQYc\r\n"
	"WCBMPxCXozAfBgNVHSMEGDAWgBTwM/RhTKHgEx8xqJBrqfE1WgLZXTANBgkqhkiG\r\n"
	"9w0BAQsFAAOCAQEAcIGeMbVk6DR2fYLfWHt8StmTCidvfLholK6+TIwhZHZwhvj9\r\n"
	"QpVsjVwfp3HerykRFR67S+NzfuQIWKDrVRN+W0cSRRq8vexwTWEE1pFXnxHRT3uc\r\n"
	"QcJA87XV8esXdCrDLR3B1+fPpLzdWld7zVsZS6c3seV+FxFfM4bFRGpmNyIPtR9U\r\n"
	"fSDihHKbkqcKW6NzFBkZFDE2/qCRzVB4fSMNbbaOB1yM9Bs85LW/SWjiIyeVB6jd\r\n"
	"ngq1tSfKRorpDWVVDSyVX8xjOdLkugVQuxoM+VchWL5t0ILSnRAUIKuXqxjrXEGR\r\n"
	"hSKZxdANCweCXiVyWKVZwCQyfQ+Np2xiAp7j+w==\r\n"
	"-----END CERTIFICATE-----\r\n"
);

const QByteArray cert1PrivateKey(
	"-----BEGIN ENCRYPTED PRIVATE KEY-----\r\n"
	"MIIFDjBABgkqhkiG9w0BBQ0wMzAbBgkqhkiG9w0BBQwwDgQIqWelqorTbSsCAggA\r\n"
	"MBQGCCqGSIb3DQMHBAjS8eZOIM7BmgSCBMigQkOm7SVA1W1rUm70QXd1X+PGym2j\r\n"
	"8mVSgBVX2gwchsAECGJgCtl8qIY1wlWK+0CCbxReoSnH6B0g8hXJuzAv1JJmd9iV\r\n"
	"RNHQNxdZIuIM3L8mxNrkzQAAjwTp4/nKZ73h/cP/D216WlOlzJHr/c9zOdx+qrjX\r\n"
	"7Vsw6+At0nxa2qaoijQ7HCuCbEu3zrCb00PkdbamFE9q+h+XUpmKuYyYAPUVhI5e\r\n"
	"+J0HjjFY7ovcjd+8ASVFbNqo9ovM1Qf/quS1+sICoPUrBwLqDcYGQokyCL0MOJlf\r\n"
	"NDR4zZaGnvKwC4BttQiSQDtxlT1PDSGAFdBvrTKMAq7H8zSy8bmBfHjvWqQf3ZPN\r\n"
	"FRhPSqV3PvIs9b4mbPIT+VT95xFeg81bv30KIsnQ0PLWbJ13EowDUtxjwWwk+h7Y\r\n"
	"CjuFdK1WgT26Fr7/DHsOFFgThwz/kV3f/cK4QMI5RB/EOCvb1/j9qqc0v4yxxzij\r\n"
	"0cKfeUx7VAlgeHhuH32aBd9PQcrbAh+azIwn8DZiKj0Wfb9aFNSAkDpOFdkuJT+q\r\n"
	"uVTBdES4MOSE+pTH8iyYECvr7Wv+5aJHLcgAxY4e6zXnGGWrZRRSMpI4ay0aycdD\r\n"
	"aXzR4Oeszt46qCbTOQlcwVqyyzd25j6c9l88hhwnagUqj8DAday6a7Ww49i6novf\r\n"
	"2KKYKEDV9pVtp61cqs592DrhuARlZcjJg+ijni+QX3qJUasHV7ggnAGuTcOQoYto\r\n"
	"Sc316QxG+CJ7exsY1gY3ONeC+2DHSMOfABE2S1wR3oENJXt7EljHIYG66PzWN/bq\r\n"
	"42EDkDEtZJUgApkxa/uT1gFoVLBlA5eMxrcz3XOFDKYwy+Gz5rffLu3qYYw4dF1j\r\n"
	"+RAiwy3da+QRdu6lSnfw1Jiu2QbYAObmspRnEZQ8k2UPMSLj/TcHgLknSL3Zd10x\r\n"
	"MlHC7lI5YycffGUdBqjieiav4XmxqI3h9qnxyGPKnaCRnD5duCRdrSiQH6DMusyD\r\n"
	"I7rqt0I7hZSjxY6i7iwyuJArM9E8+u0bKdLFkeWmfGb7Nr0ylZGusIzT6Xg5JTXV\r\n"
	"OJW7lkTOfQm8ep1LbJqsfKK4dQlYrKpYiUrZtClTkGHzPzqMfEINriQi5XhtryHy\r\n"
	"Iv6oejlaFYgL+6Z6n1eg8Jl4Key+KIJrspVSUSGRD3cpu2f+ck6NkK5uxfECzHV/\r\n"
	"uw+yNfXMZd8vZ/KKBab8bFCr0+n3bB7DLRCmn4pLQt/77mENuzxWrQombpO0ZXVh\r\n"
	"5LEJbMNRV8v36MDT+Ut5ziCpPDcbh7jOXuGXBRBhmH8GutdlvRG1Y/zftGCrdPzP\r\n"
	"lnzaJK5kkMC82135Jb+7uiTfZKLJhefIwxXKPy0R5lbXzG4tvdiQvJ5zBwhlPhz7\r\n"
	"19zBSbfwYUTVii8TRIYssM6ii0xQ2YXidfQC7JQU1ejzPON6Ai0P/w/xrPkYfW3B\r\n"
	"9mtCyawYXmZgDqQUgBF2Gov7YHHLyY8VWtliDyWibkShYapaBixCIF4JvU6iCTIj\r\n"
	"Jzu1yXoTwQdylI9fg2SwkxCV8kCeWCwYqisg9y9lbBo54i/e6HjeQ0kDfJG1Exx6\r\n"
	"svVDR0qFvOasLmpTnS7wCpwIoke8DHALRUQYzFPetjTaJ6wO3FgKadqsCY1ORyU6\r\n"
	"ijw=\r\n"
	"-----END ENCRYPTED PRIVATE KEY-----\r\n"
);

const QByteArray cert2(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDDDCCAfSgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwETEPMA0GA1UEAxMGcm9v\r\n"
	"dGNhMB4XDTE3MDYwMjA3MTUwMVoXDTI3MDUzMTA3MTUwMVowDTELMAkGA1UEAxMC\r\n"
	"Y2EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDLYRqMBU6OXox2RdIq\r\n"
	"JjvQH/h5Smt7gK72Vib+QjSdWLvfqo4f2ebUV7g3lgcyM65Z7Tqq6Qek74AMhlk8\r\n"
	"NG2s2svtMW5L4suZ2rZCdBsjXa/uWmqyZwdZRdm8BBj6Fe+wU4rVHPCNBRaZ6dk8\r\n"
	"gqE1K9H2C7jp6vWFIutjaBXMUpPwnc519XLciQWdulRGXb/vBX9HclvxW8LLwbZS\r\n"
	"LbNIU63lcckls+kR7Nvp+9gsd1k602LiXD1s9ppQIYSvg7MSYKaFWY5jSiGmGpad\r\n"
	"vH9+kJDL0Qh4c78lzLdmYpcm91pC8jBoPeLGpIHQ3jO9qk85p7oy45Lio1q9qaH8\r\n"
	"2Y0XAgMBAAGjcjBwMB0GA1UdDgQWBBTwM/RhTKHgEx8xqJBrqfE1WgLZXTBBBgNV\r\n"
	"HSMEOjA4gBQOJ44WTwL5rYvHemv+aBuXz78zOqEVpBMwETEPMA0GA1UEAxMGcm9v\r\n"
	"dGNhggkAgkUJY9mTr1UwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEA\r\n"
	"FM+vUzB5umeuWfLQX6jhxvVWeQ0bCVtlv7KV4CtPLyKVkvsYQy4aG7s/031/4XSN\r\n"
	"XCj6zMY6qifCn7wZLs5SYA0KmRjBMkpsxtfO910yGxDRVBo4BOq/BUZUPX86Dtvm\r\n"
	"BByh6WwoP9tjNchdgNpAhKFwLT+aIh1TEAAhTap84GRS+SQM3Ec3CyVg307vcRzh\r\n"
	"0gDhAtI6Z9w5Hq0SZw3IKH4Ej2d0jvmWNbVFfL06nxJFgjQ2a7MPULusvCPjDsiI\r\n"
	"wWp9pxqFJPverYIz609RV+TrTZjlsMc2MQldCnhqfZMzG89MKNE321hvM0XuP2Jm\r\n"
	"3FjTLMzu0pONcqG15rdm0Q==\r\n"
	"-----END CERTIFICATE-----\r\n"
);

const QByteArray cert2Crl(
	"-----BEGIN X509 CRL-----\r\n"
	"MIIBZTBPAgEBMA0GCSqGSIb3DQEBCwUAMA0xCzAJBgNVBAMTAmNhFw0xNzA2MDIw\r\n"
	"NzE1MDFaFw0yNzA1MzEwNzE1MDFaoA4wDDAKBgNVHRQEAwIBATANBgkqhkiG9w0B\r\n"
	"AQsFAAOCAQEAEprc1SHepabxmuLbspcZVSGVfG9ULZ0KaJlAvfods8KVbyod5wq7\r\n"
	"gWQKlTAf+t+g6tVicAGmIOF0dua0wO/4Y9kWBi4v13DuoKn91VmYxa9J/TbWcq4j\r\n"
	"FLMSNtGvfM6nGLuj13FhLLfKXjUgJi0ynhOdXd2Vv6EPBv3gTcGMLddRH53rSl5B\r\n"
	"z9Cy69JeNUvwvP2vgIFodzm1+Bsy17JT35gxyflchcX2nluUMIXM0L7GuSvzbsVm\r\n"
	"AMk5vfQxn4tXRpl7S/IgBMkQVjYOKvP5IM4Ezuh07bclI6+79TSjVS4s3p/K6Ant\r\n"
	"ruOIJYOWGK3xHEIhBUok2M7/bnowdB6+fQ==\r\n"
	"-----END X509 CRL-----\r\n"
);

const QByteArray cert3(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDFzCCAf+gAwIBAgIJAIJFCWPZk69VMA0GCSqGSIb3DQEBCwUAMBExDzANBgNV\r\n"
	"BAMTBnJvb3RjYTAeFw0xNzA2MDIwNzE1MDFaFw0yNzA1MzEwNzE1MDFaMBExDzAN\r\n"
	"BgNVBAMTBnJvb3RjYTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAO0c\r\n"
	"eca4iJrKuJU27SECJ+6ivDpNyyuzEB4vUHU87pVJNacmvPdNzypVG/0Jrl6K3hD6\r\n"
	"Ykx4Nk7OtMWXl0ysIy9/yA68+MslGltSCgOzDIO+pf/xtpWPVLrtUTF+XDqL0Pxk\r\n"
	"/qqNbSpVdOM/Vl8IgTJXzhQWuPrNbJ8MLu6bniCi0CE3HtXZq8gtZYgXYQnACppS\r\n"
	"rPvlN038eNxmawOrZw7S+X6ggFAvl+nYSWa2JbsxYktClWcWWz1XiwrRVfQbyYSG\r\n"
	"OgL0lt/MHIfgiluot9m9EFQhvgD19bazfT4O/9+STKEzhAhsvoHdC8KVYfIPYRTe\r\n"
	"VKfF2+hVc9c9AWBrMkMCAwEAAaNyMHAwHQYDVR0OBBYEFA4njhZPAvmti8d6a/5o\r\n"
	"G5fPvzM6MEEGA1UdIwQ6MDiAFA4njhZPAvmti8d6a/5oG5fPvzM6oRWkEzARMQ8w\r\n"
	"DQYDVQQDEwZyb290Y2GCCQCCRQlj2ZOvVTAMBgNVHRMEBTADAQH/MA0GCSqGSIb3\r\n"
	"DQEBCwUAA4IBAQBAdGz17r/iZHUO1Vg+NpqAGtD0NGhLn7x/V6E3U2vJpTdwof5K\r\n"
	"iGJPnEHLH30mgjtGR3DbYEr89/NaWcO9uHd/kkM5Nv2O5ZjFuemriUA39oCqHfHF\r\n"
	"Fh2yUFcecWEL2n49MLXhEgvquXRDyN04NuwjF6K4tiGAKciJVrTWduW73o9cwfN+\r\n"
	"KyUOJBSVweUpta8llZLvuW8o1X2TWu3ZZqjqlOqAfQ6ig1sE/SqLwAFDFw8+Ko1p\r\n"
	"EJ7vIEZnQNEgY/gVZ4Z4fWs5j26PWPKFMqc4emp2RxQff01S8kCUUfafT/mgWLiJ\r\n"
	"GAKxkabvHD/ZpuvUVZC1dHhCyJCToc8XqnKR\r\n"
	"-----END CERTIFICATE-----\r\n"
);

inline QByteArray detach(const QByteArray & ba)
{
	QByteArray rv(ba);
	rv.detach();
	return rv;
}

}
