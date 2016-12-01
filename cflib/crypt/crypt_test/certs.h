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
openssl ca -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -gencrl -out ca_crl.pem

openssl genrsa -out server_key.pem 2048
openssl req -new -sha256 -key server_key.pem -out server_csr.pem -subj '/CN=server'
openssl ca -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -policy policy_anything -in server_csr.pem -out server_crt.pem -days 3650 -batch

openssl pkcs8 -v2 des3 -topk8 -in server_key.pem -out server_key_pkcs8.pem -passout pass:SuperSecure123

*/

const QByteArray cert1(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDFTCCAf2gAwIBAgICEAEwDQYJKoZIhvcNAQELBQAwDTELMAkGA1UEAxMCY2Ew\r\n"
	"HhcNMTYxMjAxMTUxMzEwWhcNMjYxMTI5MTUxMzEwWjARMQ8wDQYDVQQDEwZzZXJ2\r\n"
	"ZXIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDkyD7osIoBwS7SMrfI\r\n"
	"XF4TPrkOqjrjNUqSyOrczf3zcpIEWTCgSggsAwPFHxLqwMG03QGu7nsbKucm9jPT\r\n"
	"ozquhDDY8rFXEODq5fHa69qm7dCWYJrWCwcl3Z0nh00HWTDza8fSnZRyMis7ei4R\r\n"
	"hfrjcmUQrq8F+vFsi7EFJVLZ3oFANqRgUQKBXELO2t8ovQUnmBP/zksjX0wvH0RE\r\n"
	"NJdGcM1o3/dREe5KHSobW7MVnIadSTfQQLqTgey1ehE3FLP2KRT6e1ynzdjiPWwn\r\n"
	"kqEfAQ4IEXnAu+dV5opYH+kI80a+1J7vTKDf+YelAW9lF5bZksVNgFijoHiG2KyZ\r\n"
	"ViqpAgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJYIZIAYb4QgENBB8WHU9wZW5TU0wg\r\n"
	"R2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1UdDgQWBBSShnCUqpKA9dWnjOcCs71L\r\n"
	"ukcdXjAfBgNVHSMEGDAWgBR0WcuTtUIRxo6saGuWu9VNzbpAmzANBgkqhkiG9w0B\r\n"
	"AQsFAAOCAQEABn8nCT62uS1TWs1AYbiazSCMG059AeMNo0wjTF0SKVwlHOfqMQo3\r\n"
	"5PYkEIE3joNmTMNhGXzTJXlGDNzXI4r6h2UXVGi6Kc+Z46jJDb2jlbVkMk8Fdb23\r\n"
	"pZXJaSxXAJ6B5m+8jChX2GeZ5DKvTrKeyde6CPwbqlbjcB78RYlt2oLs9nmceWej\r\n"
	"czyfk5olxodKURde+edpyUzv8WSfeAsBsdPzaHL+tkse9au7+i2Aorpdj98INWLr\r\n"
	"Q84eS92Ybu+Y8jxSJ4f1o4WXaadMVQtRwnfIrzXuCOxCAP6S5ee4i8pRdaaHVKKY\r\n"
	"lIiqFbNtnwkXHtUWA8ePDonv+0lQF3Exxg==\r\n"
	"-----END CERTIFICATE-----\r\n");

const QByteArray cert1PrivateKey(
	"-----BEGIN ENCRYPTED PRIVATE KEY-----\r\n"
	"MIIFDjBABgkqhkiG9w0BBQ0wMzAbBgkqhkiG9w0BBQwwDgQImRoZ5nfN0bMCAggA\r\n"
	"MBQGCCqGSIb3DQMHBAgf1SRGb2JXkgSCBMgxecY1Bf9Fwld/Qzaf2zNu1yb39Wr3\r\n"
	"wpxecNXkODrbOx6gY2MT9s/PYlpSe+vZrVm2qCHZuXkx2DP5ebsuZ/Zcc+rBiCwn\r\n"
	"Cmcsd/SA4MkGDHsyoap2OKSUz2p8JusXs/L+8L7GRWDG1IdZ6Twcj2NfdzVCNyVs\r\n"
	"NxjckRPDFzvjrbeJ6qbcMy5F8vAIfLMsFQgbj4wLmxNmXODdrUtn0fxoRhA1ZiWB\r\n"
	"ma6nDKOzdJhMBmRuON+fQ0n5JuQRP8lGX4Qf8Pv2XKUtE+z7ApWD6rxJx00364fN\r\n"
	"RJq6qFAZICp9kVOzBOO71S76Wv/09+bZO99/Lo1ISA9GPKVN06iar9Q/WML6yxXG\r\n"
	"fAYJORE05BzwLt9dCgt538ArGsStguxWW9DLBtZlLF3ixzeZ3GJ9b+mzYEPDl9us\r\n"
	"H3uAILKaBRX/D5+3KCfFV5X811wthG753StHZL1kYZh85HzRiBTezQJQqyjeUMaf\r\n"
	"GJyKLEhlQpz/cE1WgNkHTE+ZAFdYRujZw0b21vmUtyHeSDMRkGCBWxlwGv236po7\r\n"
	"QoR0GU8874YAIVCwZqUg4lzGxCtAffhNh627Uchdab99nvckfGA/kBEYs/dK8nbl\r\n"
	"nyCma1Sy3ITCRypMzYTwK68pObYLHATq7bb+X+bQNZbtNLmUbhydczdqwawgL5xH\r\n"
	"juetP/UFWZXakrhFCUDufpvZygbcCyvJLfSpAN9HyVbw8yKTlfI6YrtdbSeO7SR5\r\n"
	"4bNAZ5AG4ed5r4Bw+pdQyo0W0rOxEDvvIU9dL1xRDDGTBgQ0Lws1cV+ETVNxQI8F\r\n"
	"EtGTyE2JIPREaP3VgrD30l31aQW0h0i13LZczH0DfYkQ3vEpIcKqkg3Q12d8a1BN\r\n"
	"r3VD71v1WZhU6Nybuy6KWEsSlWaCzUnGf7Owe+yXxOHVi3hnb20AUC9/VVgCo2be\r\n"
	"NWKAWzuLOnZaZZUtU1vdzNdc2BIzJneYb1indRmT8x3hC0PKJ4U+UTzfPgi7+5jA\r\n"
	"VflSBTwRMcOY+Rq7mGCcxfOTfWiw3a/Gc2v68gYg2HlMH/IpEVSNAAtWDG0vbDtw\r\n"
	"QMTSJ4rmmslijMzz+A1g2ewjSnIiup16UZ9kgVwrec/xnJl4WqjkOkAmJISGlYVE\r\n"
	"ABe4YHqG9kuN3dNLXj2wHtBeATxu3vKSJVCOurvFBe68cDNL1I+BLzWepG7VsjCO\r\n"
	"wtqAYdH4PwqKbidHoJ++9za2VK1dWw+2xZyNOp5DtJ6iYSiknxXyTyVnVi4ozvxB\r\n"
	"rxaUL8q+DSdofJhNuRULSuztWQ0ka6kWFVsQGKt16bbUSou7T6w6wjMIgNCJ5FXu\r\n"
	"E3Vh/Eiy37miVBZYv60ITvcykmbQLo9+nAByGhSb1vomedDExyDhQ0M3PQ+qE4Dc\r\n"
	"589GNZZlK3ekNv1Wnv5bQwrYkQlfTroaFzOjxhM8u0dKGyl5gAoy21IMFIu5XARH\r\n"
	"JBUJySUXvfotzh2CINPRJe6A+6kzOESiQcsTIy7BPR0BXKP6nl5K4kTW4FAHbLZE\r\n"
	"1HWvC21OwEZY8Y1ZyeA3N1zOWT3piRe9uJstkQ6xgiuUk/qYbyc9VFQ5w/ldMV5z\r\n"
	"NsnY8U+i9sXKMIkRyuaE4/I5wxp8K+ILeSQLEFTwM2aaVzCbkSsyL1wvV9lnyjfN\r\n"
	"uj0=\r\n"
	"-----END ENCRYPTED PRIVATE KEY-----\r\n");

const QByteArray cert2(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDDDCCAfSgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwETEPMA0GA1UEAxMGcm9v\r\n"
	"dGNhMB4XDTE2MTIwMTE1MTMxMFoXDTI2MTEyOTE1MTMxMFowDTELMAkGA1UEAxMC\r\n"
	"Y2EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC0eMcMdWXFoFx4B+Dk\r\n"
	"A9cmUAgEH4wY7ju46rcrt7wCQDb3DGMDMPqHzH63wWZS8AmgVTAf0sItVkJzOfgh\r\n"
	"ySGg3LpT/7utXDib+leKBwhjzly5qiaEKkNISGmrXIBmjJGhZ4t3ItrZiuTW4VVN\r\n"
	"7/gdvH0ibjrWUzGZGffPhyqi3Ly5U8WL6zZhFKKfabnW3eefqqv2qlID4bfC9QKa\r\n"
	"4qiCLzBrXQLHfcp4/bLD7FluSZnKWR1OjSCRBFxK0Jtd9PAocMmiOcGIe4vT7mnI\r\n"
	"/NjaNoLE0fc3Xk1AgLeLu9SN6bNAJnMXum/vjQc5fl1Vp5PTpDxkIcjlNENS9y2b\r\n"
	"RVlnAgMBAAGjcjBwMB0GA1UdDgQWBBR0WcuTtUIRxo6saGuWu9VNzbpAmzBBBgNV\r\n"
	"HSMEOjA4gBTORs/wboVFGQGQe+50rQMLAyO6PaEVpBMwETEPMA0GA1UEAxMGcm9v\r\n"
	"dGNhggkAqELGoOQK5n8wDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEA\r\n"
	"k5voRhAO2cw/uR6nOqj1BXC6AEeeoDzBag1C/XJyKLuQzWnBUnYtPXu8Eh1R5GdX\r\n"
	"TtZwCCg3yu3VBmOGpYMxiLpz4Sa2CuyM77kScSze8q53xqtkMxQXshxa3TW/n46B\r\n"
	"z2FSvOwUNu7sZTyCUNZuhpNCOcwLbXcQlB54OhTym+HGzjsju395VqzpIqh4hKje\r\n"
	"/7M3YLJlLbaz8rW9vGgGvZhv1q1vU97wxhZ4YhXvjR3XIu9hw7sJrnjh/kh8Mw3e\r\n"
	"S+8C0PjuTM47KpfZGwLrjQ+bJ2e5qpDNsQQnCXoJ2HTUx+6tqMRJx2Z9v1s7vW9v\r\n"
	"HB1pkHR/CGvcjBWnnfs7eA==\r\n"
	"-----END CERTIFICATE-----\r\n");

const QByteArray cert2Crl(
	"-----BEGIN X509 CRL-----\r\n"
	"MIIBZTBPAgEBMA0GCSqGSIb3DQEBCwUAMA0xCzAJBgNVBAMTAmNhFw0xNjEyMDEx\r\n"
	"NTEzMTBaFw0xNjEyMzExNTEzMTBaoA4wDDAKBgNVHRQEAwIBAjANBgkqhkiG9w0B\r\n"
	"AQsFAAOCAQEAglp7dS98/PjdFtXcFQviWIKnqbHTMkDJj4ymS7mbU+WnnZH6ZYC+\r\n"
	"LlrYAk6TgWunq4B/CRfcv4NcgsPsOI89wjTbYsb+KwGQ7CAdXUnlDy2C+en7d74i\r\n"
	"zGMQYL7GcqAcY0fjKeZVPKTf72ZGQr493G6hP46FZ7lDgq0tet0Hrk/JeSGCONVv\r\n"
	"Oemcv2cyjOqD+K9FGw7zjNWq2z8mst9RTqDyyIJOwYP2YTaar012upDLU4G4myg8\r\n"
	"BszvaliGTVveg6n0mBdmedrhAQmn/jf1NGMuVVAGCzU8ZMLTtSHJFdvcyyBvxRCP\r\n"
	"vud8vG4eAnrStkiSHEDURT+0NQDKaOCM6A==\r\n"
	"-----END X509 CRL-----\r\n");

const QByteArray cert3(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDFzCCAf+gAwIBAgIJAKhCxqDkCuZ/MA0GCSqGSIb3DQEBCwUAMBExDzANBgNV\r\n"
	"BAMTBnJvb3RjYTAeFw0xNjEyMDExNTEzMTBaFw0yNjExMjkxNTEzMTBaMBExDzAN\r\n"
	"BgNVBAMTBnJvb3RjYTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANr+\r\n"
	"cp5H26R45nss3AAW55amzjMKQDkUlSMSo3f4d2nFq3yKzT5E1BwPiT8IRYoi1lDy\r\n"
	"chSvT5MUnsCulGIHI7UNBhti1UCWg02yYb30zderXMH34Y5hyYILUdhd1NqrnknV\r\n"
	"y+lZOlIj+4ElUl8ZhtwTQU59LpOHREopSm0juQuI9Zs48RnB+tyoTJuX0Jn0k10j\r\n"
	"BvI43U2F43HzPnfGQY19CI0eVynASElV3Ybb+p55rkRcrXZMoNUFn4opGtLitVZ+\r\n"
	"uJBxGss90Fz1QAHOEJ07dO81o20n6ai1gOq86gRq2j2SYRDSbe9BPveZZvSQQrEv\r\n"
	"gUFFGsp5iKamU3qIZh8CAwEAAaNyMHAwHQYDVR0OBBYEFM5Gz/BuhUUZAZB77nSt\r\n"
	"AwsDI7o9MEEGA1UdIwQ6MDiAFM5Gz/BuhUUZAZB77nStAwsDI7o9oRWkEzARMQ8w\r\n"
	"DQYDVQQDEwZyb290Y2GCCQCoQsag5ArmfzAMBgNVHRMEBTADAQH/MA0GCSqGSIb3\r\n"
	"DQEBCwUAA4IBAQC3MOMkxUSwhWPblA1gf21fjJs9ltGr4aGIVcxCaqjLWyCpDVyB\r\n"
	"ZPj6Eo9uPBlCPHzzHEohAMZ1LxsovgaYEbfApNvdwtwltWmnPMyEkdsZr8SrJ+Q9\r\n"
	"rmz9AC6ofFMi0fHQHFaQV9l2ocgXc79KJTliwT10vO7TEy+/OWs/Vkw9+20KNjzG\r\n"
	"p6MmKQWHK4Ovdf5CC3zODQvsCxQGhJ2u75dNS0dn9PitvMvsHKBtFcUST35k98gA\r\n"
	"IAR2RoWWbk3atiAX94K/AFKon0iFRoY67q9hdGs6biCFG7YMZleVzQ64C7r3rUU2\r\n"
	"f4WGclm1PAHwJn04s2avE0rXZBSsmM7ey0ue\r\n"
	"-----END CERTIFICATE-----\r\n");

QByteArray detach(const QByteArray & ba)
{
	QByteArray rv(ba);
	rv.detach();
	return rv;
}

}
