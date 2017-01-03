/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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
openssl ca -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -gencrl -out ca_crl.pem -days 3650

openssl genrsa -out server_key.pem 2048
openssl req -new -sha256 -key server_key.pem -out server_csr.pem -subj '/CN=server'
openssl ca -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -policy policy_anything -in server_csr.pem -out server_crt.pem -days 3650 -batch

openssl pkcs8 -v2 des3 -topk8 -in server_key.pem -out server_key_pkcs8.pem -passout pass:SuperSecure123

*/

const QByteArray cert1(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDFTCCAf2gAwIBAgICEAEwDQYJKoZIhvcNAQELBQAwDTELMAkGA1UEAxMCY2Ew\r\n"
	"HhcNMTcwMTAzMDkwMTIyWhcNMjcwMTAxMDkwMTIyWjARMQ8wDQYDVQQDEwZzZXJ2\r\n"
	"ZXIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDTRwgXUSPyDHDfzVMa\r\n"
	"XKoV0SVN7HHohuZIhvt+tb89tZXT5pE814mI9mBuxivRgzk9tsZo3nrhSxvUx9UB\r\n"
	"z94MrnN2BjAVNEPLuAJNnLdXTblJJ3dSNK7CfGgW6Xn6crgpM+qApJS76DHCjPtz\r\n"
	"qBW84+UFHMGl5xQQjKBntFjvcUEjZ2GCFKKl5RLwFpvhvnUFzIYCJQ7WNQ64bfx9\r\n"
	"Y9ARg/fp9sJaoBpf6fG2f3ZZwug1PAHgb3BSiXixBXI0DEmrA0DzDpLc+x1k5YwU\r\n"
	"+kNPwS25Ae2//UMp95zjsh+JWqJQX4C8MgPJVGVdDJ7QY811sf0Ha+PyiG5F6V1z\r\n"
	"lUMLAgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJYIZIAYb4QgENBB8WHU9wZW5TU0wg\r\n"
	"R2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1UdDgQWBBSRhqoa8O5Eg/jKOK9fKXf4\r\n"
	"E7J6TjAfBgNVHSMEGDAWgBSy/GMs1dRbLCeZOGZZ7zn39dQtwzANBgkqhkiG9w0B\r\n"
	"AQsFAAOCAQEArSRUETdcPep/zLpDQ4dSIl4X5+HHliqaVe8lYhdy2LaE7Sqx4WVK\r\n"
	"Tb0CT/4I8gz2Q7u63alu8hxBP7gSRlINAtqBhiWoO+rEYewNoFPPokL4yExDKeRv\r\n"
	"P9Xzld3lfm7DFrMFICDHw0DJAoNyhcB+t0FTfI6DKjJgcw78BWlQuZa5s5Cd3MM2\r\n"
	"PpI8LfWYuuL+Qe22H9SavspM49e8j2uPFNLQMuB6DBTwPebqwpJsaLuYaY7bGRxN\r\n"
	"00rxMPj+0eJQ20f5q2VeGeJPkL5APKO344kGMbHZ0YnG69Z/fmeqEFCHsug28WDc\r\n"
	"g8s+aJmf92HvVYZ7asus9JAiKb+5n99hgQ==\r\n"
	"-----END CERTIFICATE-----\r\n");

const QByteArray cert1PrivateKey(
	"-----BEGIN ENCRYPTED PRIVATE KEY-----\r\n"
	"MIIFDjBABgkqhkiG9w0BBQ0wMzAbBgkqhkiG9w0BBQwwDgQI4a/PF6+pTQYCAggA\r\n"
	"MBQGCCqGSIb3DQMHBAilO9DK4nQUdwSCBMjnLfqiKjaIBc2+2C47tWFr9IWmQksy\r\n"
	"0W+sJxHn/4llplkYay+YxfP35KQQzPbYEg88GUMCbRtAaZpMpyRIRER9a2UwLCW6\r\n"
	"/wZPeD9BcDFHip14STeCmEFCW1TxVEvfMcC/7Jy+X6c7FN5qycLLa/Iv9koFQsKs\r\n"
	"OWCPtxFelcIft/57CB94nnBYRUmUmb0HPgEx7dJP/kMebPusuzihty86u2Dpwk7m\r\n"
	"thKpsMizMMUa1aNgXtLuVWeVhaj3PYzCGLmoucCBE69bY/1AhwozieHTkgzkDo0E\r\n"
	"KUtr5JthLnhaFXvkxX7ltZpGEcPsFXQE3SPKZOyywrWs0bKIr+wshnj+di09/0YV\r\n"
	"liUiEKuuysQq4AYdmv1eCcq+/VKtuHxnjrPJwFU++pQrJ++qE3XODYOX7f6AbWhc\r\n"
	"6di34yuwLGihSVu7pSXNoqAX62Wx4NPnUWXeI4n8UtwU0blfDv63rN6F/c85wcY4\r\n"
	"1C3Tj7Zk5Ns8E6W71C2j7XceutY6FTHKl3Hd4+HyRf8vFBaSJySEXBv9Fb8qe8/U\r\n"
	"BpCcyti1LlAlet9HQb9clN5VLju/Nh6J1d4BtRWGYfvZUgzXRXiYpqlUSEqUUee3\r\n"
	"N/tGnUIK91TWfB4hs3FQkpRTUyVbTIAQTuMM1yqlchrBIEeD3QTkOfPCxDOFu1Vn\r\n"
	"d7Drhb+4uSQrh2cY0SGT5yEmKwT0psgGBFApB0CD7PzQKhsS0SX21eFqNG693Sqc\r\n"
	"it6uUhra+fC6uIor+erDrKI8AKiUjIEj40wmtM1BbbpQxkJMee5hDh6qQojLlPAX\r\n"
	"+IpAXEHBS35iBlYYdoJLKEiaCQbokjCgbRGS/cH997aUcKETf/o3mAsL8Lax6Zc9\r\n"
	"5NpdkQaEoRdEIqDiN/XOxHgqi9QWx3yDTaHSMJb/9zRKxEBSbk9g9fSesPRRFQaR\r\n"
	"rgvS26wswKEkJf8FvMJdvIAztK6CM1Hr9dvSR/2cLX2AhF7niBottiUfeORN+17w\r\n"
	"lEJp01mbzSX48pVGw4eiS+e8aMfGsGzjjVu1ZLIB3hhaJnBUYsehiekQCHCZTkNM\r\n"
	"n+dPvqDZxMwWjNcQU92aMy3pOyCDZ6qtEmKTaqMSiSNo0vLXLGul4bvGL5EweeNv\r\n"
	"bEZ/o0zd8mL4MmE8a8xsn/6D7dqh1uCaoB+BuE+8CMzY62Bbd2wAeztwIoi4Wtjh\r\n"
	"4c4WoDhKtKhNl0KNGdXWvJrJaPiYOFyJ9bcw8S8qL3h3rkIN5D5d57ehnWcexLP7\r\n"
	"7tu0cGs9c1qyDUe794u5gNkWvP+cFJhQ/E2GLoEwhmoXzrWmI59YiQ0uaow2ZiUg\r\n"
	"Bu84kTp+WWK12mPsz+V7KU8nQno40Y9uFJaNSafajzwwRBrJ1l/9ZJmH0P0lYH3l\r\n"
	"5MGYCy0sGRTwAjJJeVgttxGosjQBe5jntREskxeJ9G4jrNph5PZmX/AV4moVkrtL\r\n"
	"KzpGq9Emf6cvlwqiIoE3Lf4pnElMCrdocMHqwO3gk8SXzgo2jPCa7jklCM6oGmcH\r\n"
	"I48bXP5ol9zi9RPpyQB9zqb9jiLN0OAlVdFQ36QiAM4fScv1UnV+rlbw0Z9/mtqn\r\n"
	"ZdXu7y7m/L2jYeCz+B9VInfEw7SFdPdr90P0O1iDaW9SlHhvg8Z4rucXV1d90Pt8\r\n"
	"t5g=\r\n"
	"-----END ENCRYPTED PRIVATE KEY-----\r\n");

const QByteArray cert2(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDDDCCAfSgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwETEPMA0GA1UEAxMGcm9v\r\n"
	"dGNhMB4XDTE3MDEwMzA5MDEyMVoXDTI3MDEwMTA5MDEyMVowDTELMAkGA1UEAxMC\r\n"
	"Y2EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCurhvgoBTbplOCdBJR\r\n"
	"HObD1one4XX7JoKtLjyST8PsapCvpRVyDJX4+9ySfbp439wM6h4HyynwSDfYsTtH\r\n"
	"d8yoXr360OuMaft5gMh7O3ZMsT/m/ge9lP6WbcZmgDJI6wE7JCoTAWsBRhRnu816\r\n"
	"xZ8VDN1BRF0tFGY3wxjBmaZGfgux0iz2Jknd2UUquSRfhnvgy5sut1KP+JmpomE8\r\n"
	"xSBphqXRS8jigltpu7CekFxdcsyomKBIAp/+OHdPbQnh8HcmbLH7tndQr3Y3NUb5\r\n"
	"K4lnxKeM3001gvbppJVghzY/ffg5PbY9VkSMtam+jkVNJFtl1O54uFxrpGlKlfho\r\n"
	"tnNVAgMBAAGjcjBwMB0GA1UdDgQWBBSy/GMs1dRbLCeZOGZZ7zn39dQtwzBBBgNV\r\n"
	"HSMEOjA4gBTv4ykJs370B5O5hQYgmuUbg4tAq6EVpBMwETEPMA0GA1UEAxMGcm9v\r\n"
	"dGNhggkAuWd49X+SUJQwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEA\r\n"
	"gVsYmCWZEYf2U5id8rKIWeNL41Meq7I3ADg4hRCXUd95Y1jv4T2bLyanHnsYU+D5\r\n"
	"f4g2VgK+td+fz/TXLDYiCLX9enTXbgdOmqx7zMgbjgKj66IeCyTKHKcCvJYKPXof\r\n"
	"X07PGoBo0vF5L6NNHvpro1ga0Vt19ro/DOQWh8lPFzbuNU/1PKqU9Wu8JHW5/cQl\r\n"
	"5QRgXcRilBAeKaOUbz6LIjbV4XDz2MGjnnEH7mv8TjiZJnoTHWWSDz/95DILU+cY\r\n"
	"+loFD80SQhugQ3AS9Rajgc0jI5oO8pTBFslZ6+VVzCZqX2S0F2F9Rzh5JLNmanNp\r\n"
	"hIbu8eLJ0AQ3oONs79/sUA==\r\n"
	"-----END CERTIFICATE-----\r\n");

const QByteArray cert2Crl(
	"-----BEGIN X509 CRL-----\r\n"
	"MIIBZTBPAgEBMA0GCSqGSIb3DQEBCwUAMA0xCzAJBgNVBAMTAmNhFw0xNzAxMDMw\r\n"
	"OTAxMjFaFw0xNzAyMDIwOTAxMjFaoA4wDDAKBgNVHRQEAwIBATANBgkqhkiG9w0B\r\n"
	"AQsFAAOCAQEAPj7XMJ2KEzIXTLlmh/Fk22VArsIOYCRmk2hJRlv/Ld1aVdxE7VIZ\r\n"
	"fjXLk4QkRt1G3Qy5kjOoPT71Kvtbq4KCfG/jGdW7kSptZwyHTsCnIvqf9I8LJmly\r\n"
	"Ptq1o8oIxzV+EOvtmWGT0rbzM5OVpDqUYH+SbitY8Zx9mMMLburnNc6fKAAJ5F9a\r\n"
	"UVRHNPYUdtbcaMhIls1vc1KulHJgDHURCl3+oK5VQRzOqtv28ARyae75Q//rS6LG\r\n"
	"txXjntTmGsBl1zVrbtN8G50E4ub4vglKsyQZyC3vcyTEA6/Cr5NCP+crczlfi8oJ\r\n"
	"zJRShK/CjzyTSALOFzdNC+ftHp+tJJdg6w==\r\n"
	"-----END X509 CRL-----\r\n");

const QByteArray cert3(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDFzCCAf+gAwIBAgIJALlnePV/klCUMA0GCSqGSIb3DQEBCwUAMBExDzANBgNV\r\n"
	"BAMTBnJvb3RjYTAeFw0xNzAxMDMwOTAxMjFaFw0yNzAxMDEwOTAxMjFaMBExDzAN\r\n"
	"BgNVBAMTBnJvb3RjYTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOYN\r\n"
	"p1xiDdc+s0T1NsQ6yP/u7HaA8klR+jdroqzy+eBT19XIkCdG+Dyp8KyA/Nlwwdb5\r\n"
	"+nLoqjS2eD3FvWLs19k1GSLzaBZpRJtYlLifPS25baEsYLC18gbyonQQ/z7xDUwX\r\n"
	"Ri8vjaombXlD2y6Bwx7KrIH95uNq0LYThYFFs0SZSwT8MS5vrQCUTeYztr+GJpUG\r\n"
	"C2HabZqp01B3g360sEqTNFa/OZn6+hWHW6aZvMc2TO2tyiH8r2ZAvOy+2a+ClU7G\r\n"
	"03UHjjPXOPzg7TGy1IflXZNsEt7Kimsp0GuSpCaUS0r0ccHCzMvVJu3j7LXLcBM2\r\n"
	"jFWPqRHbYyS/CQZzfdcCAwEAAaNyMHAwHQYDVR0OBBYEFO/jKQmzfvQHk7mFBiCa\r\n"
	"5RuDi0CrMEEGA1UdIwQ6MDiAFO/jKQmzfvQHk7mFBiCa5RuDi0CroRWkEzARMQ8w\r\n"
	"DQYDVQQDEwZyb290Y2GCCQC5Z3j1f5JQlDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3\r\n"
	"DQEBCwUAA4IBAQBgKhrJdbX42MkFL8H/gNctqJxJEIqMRwcQH7bT46g+PdvJJy+m\r\n"
	"duqHGXPFfdIH+oIzdaG3EuOB9qETBSGjFaQ+zkGRS1sO6wFY/XlS0hZG8jIFbIBT\r\n"
	"s83epiqAeg3XbBrgzy0O2zqZzT6vO6DPU5pJkUUvjdlvVMonKeHN0KwdsidsV4R0\r\n"
	"8h48sYM38Hsw0p2rphswtpnrkZP7ReHtSeCzg59P9Oe1wVPr2HtMWhPqV2yk3w7e\r\n"
	"77Gtf7tdivFHl4ha+rinPL8+aqBzvym2NvZ7HgtNdSUOhjhG4yDiIK6QsU+TzpIj\r\n"
	"v2WWQpdnQl1vCNMDVqbf8TKKZFk9qmK0D2+1\r\n"
	"-----END CERTIFICATE-----\r\n");

QByteArray detach(const QByteArray & ba)
{
	QByteArray rv(ba);
	rv.detach();
	return rv;
}

}
