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
cd ..
openssl req -new -nodes -x509 -keyout rootca_key.pem -out rootca_crt.pem -days 3650
openssl genrsa -out ca_key.pem 2048
openssl req -new -key ca_key.pem -out ca_csr.pem
openssl ca -keyfile rootca_key.pem -cert rootca_crt.pem -extensions v3_ca -in ca_csr.pem -out ca_crt.pem -days 3650
openssl genrsa -out server_key.pem 2048
openssl req -new -sha256 -key server_key.pem -out server_csr.pem
openssl ca -policy policy_anything -keyfile ca_key.pem -cert ca_crt.pem -in server_csr.pem -out server_crt.pem -days 3650
openssl pkcs8 -v2 des3 -topk8 -in server_key.pem

*/

const QByteArray cert1(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDVDCCAjygAwIBAgICEAEwDQYJKoZIhvcNAQEFBQAwSTELMAkGA1UEBhMCREUx\r\n"
	"CzAJBgNVBAgTAkRFMRMwEQYDVQQKEwpDQSBDb21wYW55MRgwFgYDVQQDEw9JbnRl\r\n"
	"cm1lZGlhdGUgQ0EwHhcNMTUwMjExMTExNTE5WhcNMjUwMjA4MTExNTE5WjAUMRIw\r\n"
	"EAYDVQQDEwlzZXJ2ZXIubXkwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIB\r\n"
	"AQDP3aaEFYkqbO+OkGf/rQigeE3TRZ0uds7UraK4f/LAVGBNGSsKXdK70hK/GTIx\r\n"
	"2Z7sgIg2Ic+MsI6ePamBrHFA6/+0zyFzIe3Pl/ecr5T9x2gE+WqWSRK0MC9zG/M7\r\n"
	"aMZ+KKpyDcywB4vTaTR9NmyxDZxnaIMVudp2xQ8AMNdDh0BJ3NBdjWPTuTn17xTQ\r\n"
	"2wM+7x0E3QMg/UnQZ0jGkrw/3z2eJJLGO8nPCJD3aJQeobcTvbfbJSmiUxzPHcCg\r\n"
	"5pyH8rIGGW7eMXrn4Zl2Q/Vr8Uf+W+R+WaFeuATbmDlrar7pmJfK4OyOguwoFNQ6\r\n"
	"cW1m3rV8o4mI+nSV1+yVApGXAgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJYIZIAYb4\r\n"
	"QgENBB8WHU9wZW5TU0wgR2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1UdDgQWBBSt\r\n"
	"p0XmVf647AGVGJLv/R2Gss6COzAfBgNVHSMEGDAWgBQU7F0hlVcdfjmDbG5UGBL0\r\n"
	"kl/gQjANBgkqhkiG9w0BAQUFAAOCAQEAEScdtJr1ELaMJgSFxKQP42hf/mXTMzX3\r\n"
	"NeCqthgrN4nWal7tZBQxW7cgtSqKtMIgkmskREmRVu7wZzKaar61CTwOHIoy2b9+\r\n"
	"UQFVf6/3xjl6egu/PlPZQiwwsl3DJyCH5bSl+0cBaoXpPsp5ppw3SABYiHt4Wv3o\r\n"
	"F+8cnuD+XleVAqJOZpPL1eiwWbZlPzziyLkN7KkOEcnj3RMVOLcAl0CXP5K/urQs\r\n"
	"Ry57KS7aDhqCtwDcPzJ8IVz6XrqSN2uVwbIBbIGPbddcH0YSnjP4E1UVkISabobL\r\n"
	"hjoPlzLY9ZhU5ZZRL05CK12o7YMxN6AJ35nCcbqRHM6i/vf/lck9sA==\r\n"
	"-----END CERTIFICATE-----\r\n");
const QByteArray cert2(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDKTCCApKgAwIBAgICEAAwDQYJKoZIhvcNAQEFBQAwQTELMAkGA1UEBhMCREUx\r\n"
	"CzAJBgNVBAgTAkRFMRMwEQYDVQQKEwpDQSBDb21wYW55MRAwDgYDVQQDEwdSb290\r\n"
	"IENBMB4XDTE1MDIxMTExMTUwOFoXDTI1MDIwODExMTUwOFowSTELMAkGA1UEBhMC\r\n"
	"REUxCzAJBgNVBAgTAkRFMRMwEQYDVQQKEwpDQSBDb21wYW55MRgwFgYDVQQDEw9J\r\n"
	"bnRlcm1lZGlhdGUgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDf\r\n"
	"rEjmelQYBBKY1tBQRJezu7NLk1f/eciMREdokm7u/UXQpZHTKDltwmXPzSFAnwFD\r\n"
	"T4l5ldK/6w7tXdX0z60PaRS2+kaU2voKYmTBjDNaVNKoR32+QDpODSpPz/ymcbvK\r\n"
	"vgdYAcQ86ijlC82F7dLaQBEQpAxnTX0BvInaIeetdO+IL8ZVXdvHTWE2cg/T69jx\r\n"
	"HBY9wg87cP0mUS+UdDzKAIVhaEvdnXTGRTlI9aFXe0PKYv/03lPCUrLLDc5nS4Li\r\n"
	"o4iyUJQ0Lhx27NcP/jDmNlJ/68XbGZE+tW1h1yiS/6Hx46nBjhgc191lygilIqua\r\n"
	"P45kSgXDO5Gc8bTDU8TZAgMBAAGjgaMwgaAwHQYDVR0OBBYEFBTsXSGVVx1+OYNs\r\n"
	"blQYEvSSX+BCMHEGA1UdIwRqMGiAFC4SO6XRWBNBOLbNG8yv4gQs0cmnoUWkQzBB\r\n"
	"MQswCQYDVQQGEwJERTELMAkGA1UECBMCREUxEzARBgNVBAoTCkNBIENvbXBhbnkx\r\n"
	"EDAOBgNVBAMTB1Jvb3QgQ0GCCQD4jz1phbfnTzAMBgNVHRMEBTADAQH/MA0GCSqG\r\n"
	"SIb3DQEBBQUAA4GBALgPNnK+70K1mcAJAw+XELSsRwc7i05RLNp7pcb93fxjzRbt\r\n"
	"zTjW+2dPvcynvMnLpzjxMsK8L5o9AWeoA0EeV/HT9aE/Oz9ztt+wK9c6yMvWVw+F\r\n"
	"YGyl2h7QfBkhW+HR83OEpbGwKguNBt7Xs9a7qYNcDVWzNVm5IPfPLgRkL8+w\r\n"
	"-----END CERTIFICATE-----\r\n");
const QByteArray cert3(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIICpDCCAg2gAwIBAgIJAPiPPWmFt+dPMA0GCSqGSIb3DQEBBQUAMEExCzAJBgNV\r\n"
	"BAYTAkRFMQswCQYDVQQIEwJERTETMBEGA1UEChMKQ0EgQ29tcGFueTEQMA4GA1UE\r\n"
	"AxMHUm9vdCBDQTAeFw0xNTAyMTExMTEzMjhaFw0yNTAyMDgxMTEzMjhaMEExCzAJ\r\n"
	"BgNVBAYTAkRFMQswCQYDVQQIEwJERTETMBEGA1UEChMKQ0EgQ29tcGFueTEQMA4G\r\n"
	"A1UEAxMHUm9vdCBDQTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA6/N2Vbt0\r\n"
	"4CxDk933GHHk8A2J702l0sP801y9rYfdyidlzst5ilO1142NtH0BqimRWNJjJxeo\r\n"
	"BmEOm3K5Q7QduLNOWL/K68NpHg4DAOctCvczgRS1p+WLYCar3v2Y2LpzIqHCSvoB\r\n"
	"V/isKPoqYN8oWk3vdvEykUlktxokM8DCkukCAwEAAaOBozCBoDAdBgNVHQ4EFgQU\r\n"
	"LhI7pdFYE0E4ts0bzK/iBCzRyacwcQYDVR0jBGowaIAULhI7pdFYE0E4ts0bzK/i\r\n"
	"BCzRyaehRaRDMEExCzAJBgNVBAYTAkRFMQswCQYDVQQIEwJERTETMBEGA1UEChMK\r\n"
	"Q0EgQ29tcGFueTEQMA4GA1UEAxMHUm9vdCBDQYIJAPiPPWmFt+dPMAwGA1UdEwQF\r\n"
	"MAMBAf8wDQYJKoZIhvcNAQEFBQADgYEARbpOZDJvjN2nIsObJcJGLdMgbHGJ0Cuw\r\n"
	"eW8QagnEJm0ogpw7N8XMdbY0wMtpiYBmdp0kK0BadEPL2HwCC6+LQ3Q2teeT99Ga\r\n"
	"UsE7oGqbHfs6Gc3PLkH0VmsGYIQlPm3QU1yKdpcrL4+h7gCYMRZzcfiBeJTW1Xcc\r\n"
	"yKLVxpos0og=\r\n"
	"-----END CERTIFICATE-----\r\n");
const QByteArray cert1PrivateKey(
	"-----BEGIN ENCRYPTED PRIVATE KEY-----\r\n"
	"MIIFDjBABgkqhkiG9w0BBQ0wMzAbBgkqhkiG9w0BBQwwDgQIa7dWgM+f7PACAggA\r\n"
	"MBQGCCqGSIb3DQMHBAiTgm3pyXwslQSCBMg1t4Ms4Gl7Ps/5Nc8ORSkqEsNoZ0qE\r\n"
	"b1c7n0Aqwb36wlwtJ3JtI4eg2d4f9c3mvqGQCEd8MW286pdXdiqXyKKVzvZ1R6J4\r\n"
	"rIdJCzTRb2boC1uBEzOozRD/pOvcp6BFFZo1LgeZYudUW2A50T1rgsGC7IAmVj7G\r\n"
	"5KZJHf0Ps5ubIiRV1jiLiDt420rElejYQxm4AQWJ0Y2O/vAuJgjZICCFondpv+Zm\r\n"
	"ER3n7eFlPssEhEJldpzi7ckwJ13E12IuY/5PegnseuaAWfhOs/+7rpXNWRIAezEo\r\n"
	"KMcxGzlXzYNp9oipX+qrd7lzD9dmiTxs4/am+Gd5PLWU5dTDQDs395C4kCm58lbc\r\n"
	"h21evg33umNNbMwP9MEh4fJaqsdnPRdRTMfuOu4GZt+NSdlFVSMYTQKXI+ftPMeY\r\n"
	"clCXhBJ+b6GTwUS0hYBkSjmbHBzf26LCNmg2zClhuJx85KnJ4iqp2wFGlsEA09yf\r\n"
	"FZYQCiFIulElDwFAbrpoEvtwc+hDz+KQFaeCGrJceQVsSJJVlr/VyIFXNFyQiePt\r\n"
	"8irZwHAbRLBzHJqrqzef8fpfTP5tSKo9cGJR7UtEjzl58i/9CTcdatA7giYUqmVM\r\n"
	"BMCDsOrB28ZfrJUT94SUJ3MSyFBmCtyEzCiUZpbWd45PnbR75AiRXCn5zhGYad3k\r\n"
	"HenliTDggK46eVrw/ZC+GOA51OBl8WiK0OKdlFxCilUl54FaKIztfP3x0JLqYddo\r\n"
	"Y6vKCF28pz4Q6Uf2p5+Wm1DZIj/xLwPLRcScw+Xuy1fNg1mG0ljDCnxV2eM9wgVS\r\n"
	"nL/cPS5/a6g5QagWK05aSpze6iHtLyFmSJSGchp/PzpJKUX1icjRjB6s1Wyw78oC\r\n"
	"GIVp7sbXKFuv0rr76Z++n8M8Ogd+OdAb1k+xGl+X+TjlTKfayI5bNAD7ZkQLO96R\r\n"
	"KNvT83z2n3v41PXu/fc/ZnXgDy5RqyZBAXCkm+z2DPORwgJFYSMrfTJ7H9PYRF8+\r\n"
	"XyNdLZ8vn9/sHYeZpjqhBuTooi3hhN99r3pw8Ppz9aA+CRtHwrNrWHEs7beCpGnA\r\n"
	"WUIVihpKvqxBFE9x97ScyoyAKO1tGcWOsKEXN+Jbhc0hJ6UrUlzB2mwyQa2iWNmM\r\n"
	"B1iUjIeFzkDnUlKa7+DE+15Xd87VR97+ys1Yg6bU/T8LzQDJ2URvOxrBkBjEe6SR\r\n"
	"J0riU+8OGHAjIr2n+3Q33nJ26F1gxJeRSRsltDANJfQlAf6f4e2szEAuJ2THODnt\r\n"
	"Hc940ux02f4pRh77XHRhMOnVPuc9Ig/MMR6wt/vTwrnWiqjbF7pDqoU4IH3Ik6QH\r\n"
	"RQtCAe614oe5oky7bJgpPdCC6Ojj4C/VyTo/ZPzQDkrjFjjffSmpWYfF1dAC3DCb\r\n"
	"h62axEu5ajvfB8GTG9N7CNLJ5QbNkCmv+3SGmEMvV7UyQS9+lDYg6av6iIBX3ha/\r\n"
	"fKXKFpOz9gq8hQq1BXHh/G4jLBLLyY1XbmFnOOsAcZQhzcV+CuHvYqRNzyJj4kf5\r\n"
	"TUJy1XEcrL1HPKBuIXmoI5rs6HA2KmmM6bOvFdQkCZrjTCXArs2UcMP1tTszBUE5\r\n"
	"hKG5dDz9fNX2Q0kvoM+19F1wjqe9R1nmMm6Hk3Fi61LcGcAsKW3650LMGLBsfrhv\r\n"
	"muI=\r\n"
	"-----END ENCRYPTED PRIVATE KEY-----\r\n");

QByteArray detach(const QByteArray & ba)
{
	QByteArray rv(ba);
	rv.detach();
	return rv;
}

}
