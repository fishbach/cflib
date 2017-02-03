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
openssl ca -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -gencrl -out ca_crl.pem -crldays 3650

openssl genrsa -out server_key.pem 2048
openssl req -new -sha256 -key server_key.pem -out server_csr.pem -subj '/CN=server'
openssl ca -md sha256 -keyfile ca_key.pem -cert ca_crt.pem -policy policy_anything -in server_csr.pem -out server_crt.pem -days 3650 -batch

openssl pkcs8 -v2 des3 -topk8 -in server_key.pem -out server_key_pkcs8.pem -passout pass:SuperSecure123

*/

const QByteArray cert1(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDFTCCAf2gAwIBAgICEAEwDQYJKoZIhvcNAQELBQAwDTELMAkGA1UEAxMCY2Ew\r\n"
	"HhcNMTcwMjAzMTM1NjQ0WhcNMjcwMjAxMTM1NjQ0WjARMQ8wDQYDVQQDEwZzZXJ2\r\n"
	"ZXIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC/D/yE5smkU2E2QgT0\r\n"
	"z1kvMFAV5WZOxTe64bbHEanrdGGAPlz1HnXLfpJ3QMgGnL8Xcge0RbgGP7giN/Em\r\n"
	"U9hQaZkEf39D6S4Vwbm0y9YSx+OkrP56wXGuatH0+vx5RAAQ2s2yP1CThPag7oJT\r\n"
	"9JR5W5DgXzLH2yIPMaAdtR9/RNmiWsJDmZJRoxfG+8TK+PTTHlshzzXIUOnEt/s+\r\n"
	"DIIzGu74w4lFbCXz6m34xMcyk2oxx4MwXZv1czceiqTIPGBd3e2w+yYUW+h3XszR\r\n"
	"0DWC3a/4CoEianBSHu25yMBhSbIvGbjid/mH9JHexoIOdS0hVqybBVHZg3G/qIan\r\n"
	"6QMdAgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJYIZIAYb4QgENBB8WHU9wZW5TU0wg\r\n"
	"R2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1UdDgQWBBTNtL9Z35qkva3yRFRuSlS7\r\n"
	"sb/X/TAfBgNVHSMEGDAWgBTV5J/TgZxwcXcwYkXKuVdVmhKv1DANBgkqhkiG9w0B\r\n"
	"AQsFAAOCAQEAtAtZgog/xdtdfDmEDLa+eyY5uIZeuba10PsG2iU5JCOKLPxRGWh6\r\n"
	"k2iIAYt4Eos6QzvhZFFR7uo/DoDKzKQGEqd5N5LJfPSqdKyPKXUSINvKf4Ix5d87\r\n"
	"nHqRvoMzpCP5dWe4iIyqqvIRUSxCp0sN9Cz7k92pDTcC8LCr0Yxl0wfWAg1LYhE1\r\n"
	"gupLh8uLe39D2z+XdCaPBegX1wYr+shuey3+FHU4jMOqbSW4leII13ZysvVFFvUj\r\n"
	"8Xq8OS+XPWK/YREdSjDTWNzguaQFSDsVywgRsqY6R69bBG16JYornzMecT3bEL1z\r\n"
	"+zTKTWBCKXpEWCNyb0UFnpDFJhmmiRHgbQ==\r\n"
	"-----END CERTIFICATE-----\r\n");

const QByteArray cert1PrivateKey(
	"-----BEGIN ENCRYPTED PRIVATE KEY-----\r\n"
	"MIIFDjBABgkqhkiG9w0BBQ0wMzAbBgkqhkiG9w0BBQwwDgQISHfmhMs27K0CAggA\r\n"
	"MBQGCCqGSIb3DQMHBAiav/8qJ3v7OgSCBMjhRN1CnTWFATN0h3FUGqUo/aq2nXLm\r\n"
	"dslptVhCI2376fp6qgldr2AQwSblBIi1lY7AXV42cr3+KrsiN8oKVW/dPl3q7JDf\r\n"
	"H+26stlfFCaq4vsAnhNPkRDpm86+ETTXx6JYbXoQnrGwl7bOUDUzh033XPcFUysC\r\n"
	"+3qEdlcY4qnLROgEvmG6xssm7sHJwnFxY8YD/zAnk4uTnhqIpUk58umMLEY20e5H\r\n"
	"M80V+DgpgsI6GkjxzN2nAHcFc/9phAvvFMmryKKu3koDKxhMdO9RXweGW1iN7Ubx\r\n"
	"nEIBgv2nVRGNmZxNMFnCzF1pbQx7odvJZAgMj8LgDtK+Hj+P24i8hlGUjreqq4Dd\r\n"
	"0PPLUkUskcj3N1hk+biFMCs70j2MjnVxumrMbhh2bbzpTPr3iLQeVgsCxRxIAlpz\r\n"
	"W5vjZrPzj+6ucCbcUB3z6J6MnPIKaKm8Z6mk+sI8QjkgllFrMAroa8Irz7zBe9uQ\r\n"
	"bSBRKWj7bgWtJEVV65B1IshwSLL2vHPl/NHq8XcgxGMTgR9H6eFpRrm6IDuSAZ+0\r\n"
	"O11ka8ZAWyxaFyq565JgA63TI6Z8Y5zP2S83Y/nAvlgGZgyjJ3uJpaVQiJ/CsPaf\r\n"
	"ASb93yv6gvisuCUFhAXgQJO+U/dJULsde/WMGbJbtwMSXy9+T+7Pw6QMK6dS9sc3\r\n"
	"TRUOM8OO+q21FgHITcqJF6rY6SkN6gVvj6VKKQTZVVchFnUEIN2qeiRAYQKaWdCb\r\n"
	"pIsXF32eO/CXZM/IeyVkGCRFVUQFHnk/ztyAAVuJGIAlYZPa997O7NbhdLepIlIJ\r\n"
	"M7a5mcJWnZHTrVhGeMSBK6Wt3KnCnSVPkO1EyYklXGLUgwkQxC9oL0aktwUmdA5Y\r\n"
	"XXddOnCVrSBhC5LYZBGll0VcjYA/gqFAIih+pR/uyBjUs3S6J6hNE2IokhFmE1uC\r\n"
	"BxZWSi/++IH1yxl5o3lNsDBj+aXW6kP6YRT6ZlQZUtnU2D9BGxB8lo/UwXlrEftI\r\n"
	"bneGIZsm15RbUEz0RBaewUVA9SjA/BjBsnoM/XSR3sDz6YhnZWEYDJe4cK022VLz\r\n"
	"H83Pgy2BKHGxzGq36JEqnrGxHWFqQg96o+zUp2u/M9cz04+SGdXUZLGERvMYlpKH\r\n"
	"NrD3UD2s+9wo05SK3tctZcwdVdp/q4uYFfpmDczCEzSuG0f9Dyb+br9ipYKivMny\r\n"
	"e7OylMgeOUPlZU22RZWRbuWEElxmLOYFbiFbefO+RG1+TTb/gBuStg6o/nvcuTbG\r\n"
	"vuvkLKYNPd0H1bhqXY84toc/FBH5R/dKgR9px/1QOB0IsaI+Cd69oBbN/QDWS2mo\r\n"
	"XIKhXLeHUpne2l6e+2LuAleBaMj+M6VSYp+YbksjOvK4mlONZU/oLV+UxUdLmuFx\r\n"
	"Gyvz++r2CfczVXE88kMNwtx/egCqefb9eGH2darmxkSNndLHrmPClrp30AdICGIq\r\n"
	"saJPteLKoynY631o18oc4si2seUdMbWccXNG5zHaTlT8QeWkLPzchH5Y5+6GBk/H\r\n"
	"B19LvoJ8+sZLtoVcvWRFqQ/yD3wWka9V649F98IEKCcHDzuhw+sqHytZqCkctHah\r\n"
	"I3qUp9qRwOUIYfOPS0//88IRdaEFBqOYQoXC8ML4WzYlmdbczpoBAdQJsbOhYerb\r\n"
	"m88=\r\n"
	"-----END ENCRYPTED PRIVATE KEY-----\r\n");

const QByteArray cert2(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDDDCCAfSgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwETEPMA0GA1UEAxMGcm9v\r\n"
	"dGNhMB4XDTE3MDIwMzEzNTY0NFoXDTI3MDIwMTEzNTY0NFowDTELMAkGA1UEAxMC\r\n"
	"Y2EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDA5r/imQuLExwtbplT\r\n"
	"e5XWPYKKcv9JJhoOC7p2iW+rKu9bF97D9ePNQOIVatZITlIoxxp4e9awJVHzeWuY\r\n"
	"9r983WkNhSXcTsFvSnvHK7U2QmuxsU5rcvtBt+Hlw5krXCzW4oSMbzr6fjsfHT8+\r\n"
	"pUtSiwl67ihkop5bjPs78IRvt34KA+bzzXwB91j6qUdscZo/dDLB5s3gTFvETcHe\r\n"
	"jEEnX83sgzP+RiGvO1fOIYxdZygurFEojJh/5Y64dup3WHRTX3D4dzcAVWYp2W5m\r\n"
	"HBS4JZGLV/oTW+11WbRFrjZ3JiJUz+x240xy2lF4GeuFtN8nj56SsaAu2nAi+8UI\r\n"
	"S0rhAgMBAAGjcjBwMB0GA1UdDgQWBBTV5J/TgZxwcXcwYkXKuVdVmhKv1DBBBgNV\r\n"
	"HSMEOjA4gBQmFxH8txoryI5xy2xfcllzdM+twKEVpBMwETEPMA0GA1UEAxMGcm9v\r\n"
	"dGNhggkAqCu+90E0wLswDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEA\r\n"
	"p+qU2vZSBtiPdHKcDFogbmpBx2VRBGfSY03mQsx4c0Nbyb74D7FG4psWf8P4d/MT\r\n"
	"YbpImNSsftJurWkBPs+NOXdNGRNRHIkc8grOoGvs03nwLp2et1KOfyN+hZOS5gRy\r\n"
	"S7AXilpjQu3dRxDlGlu6vYbiE33la7TpQgfGPRGQTIhHEubm+Jvx4INrheJqWcO4\r\n"
	"/rX9LeFpfr7X3MN3LKea3p4jY9ZiDgprGhQ9PgU1zm4rvJTmYDCJh/TpgnM2QOPb\r\n"
	"gD2H2wEqfdD1v4q+5ihq6E4oSR/kDL0VzgK4WG0safovEG+pNzSTdok4VKSYJ0D1\r\n"
	"FrfVIP2vx4wIzo8TmFLscA==\r\n"
	"-----END CERTIFICATE-----\r\n");

const QByteArray cert2Crl(
	"-----BEGIN X509 CRL-----\r\n"
	"MIIBZTBPAgEBMA0GCSqGSIb3DQEBCwUAMA0xCzAJBgNVBAMTAmNhFw0xNzAyMDMx\r\n"
	"MzU2NDRaFw0yNzAyMDExMzU2NDRaoA4wDDAKBgNVHRQEAwIBATANBgkqhkiG9w0B\r\n"
	"AQsFAAOCAQEATl1DN1cQuRi42OAcqQNKq+860NRfEG03IQ/kKr5XkBVw57tMk908\r\n"
	"9Ljsn3fjOs3dn3Hjy91hb9kgcwm2SSqSRYomtIZ1r0LnlipcZwhP099v3xbNYZRN\r\n"
	"bzF/q//kvA3X2Lhi9wLtwkvkGl6JUPJ0I4hWdGH5zKHGhnifkBytayiFj81aMPa0\r\n"
	"qhVe98wPo5PnAZ6wIccWO6g2HgMEgoSQN7Uyk1mMHgryTJlXOxIXVThCTtx7K0nh\r\n"
	"iBoUDnCI/XkTzVcd//Oq1o/R5GoxZDauwGC5Se/WvKpxpgwntdVptZdix2FcWGrf\r\n"
	"CkZSqBQWkrOsW1UyDv/caqTFEYpZ28+UBw==\r\n"
	"-----END X509 CRL-----\r\n");

const QByteArray cert3(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDFzCCAf+gAwIBAgIJAKgrvvdBNMC7MA0GCSqGSIb3DQEBCwUAMBExDzANBgNV\r\n"
	"BAMTBnJvb3RjYTAeFw0xNzAyMDMxMzU2NDRaFw0yNzAyMDExMzU2NDRaMBExDzAN\r\n"
	"BgNVBAMTBnJvb3RjYTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMXo\r\n"
	"9sTnqPJo9DfeoNcezttV9KHw4T9ISvE5nZTSt+RpnwhaerrpFMjS8Q03VdafXcJF\r\n"
	"B3z8GNkZGzBoyX1Nn8Iq77u3LIpfN/HduFoJRu/UudswpS+2Py/nENUl4ljkptHm\r\n"
	"wWp/yUSm23ZpImQSJZOewxfFqWk613gFXUabImz9LUKmt0jEvgeOdIhdsnPurYuA\r\n"
	"o/b8KTeuLQVMKHY2iaOg62355BmDN46nQuB0YKaQNID1PCMlwHUCQOK/zlrsQlZP\r\n"
	"pRh/FBiPtNR6e41tVOSI0thLGqVGKJCQaSt7Sl0Ecr92C6lZCDAXN6OHgIGL8VVc\r\n"
	"CqFoXfF1vh5SwhoJ1CECAwEAAaNyMHAwHQYDVR0OBBYEFCYXEfy3GivIjnHLbF9y\r\n"
	"WXN0z63AMEEGA1UdIwQ6MDiAFCYXEfy3GivIjnHLbF9yWXN0z63AoRWkEzARMQ8w\r\n"
	"DQYDVQQDEwZyb290Y2GCCQCoK773QTTAuzAMBgNVHRMEBTADAQH/MA0GCSqGSIb3\r\n"
	"DQEBCwUAA4IBAQAoSJQHrVtvEpmOPl4J5+LLRszPjr+5AJ0e4+vl4mOf+5mylkYe\r\n"
	"AwBKbCD1HRUoytcriuTNXTr6A3kFfUE8O4uQ5CJNM1JSKmHDam0mscLUYOVWwRve\r\n"
	"UEwcuhgfqP0nKPHGc7LR0fPt9IdU89Bx+vQI4QfJ4rVLYkvdp8APjVomUiVCpgEp\r\n"
	"0MUswIoY7BCmaz08uOO0KFpUKubCCfxtJpqEe2RFWOIlyAu9+S1Yy3eBESe8caTY\r\n"
	"qfUxYDa2KoBGeQ97D7cZDuNp5X5rQZJKm/OEediem0Ku7rqMHiwAMbg0960IVTZ5\r\n"
	"VcFEYpze4fVS3CZEkUHSSekkBM7IvDBmpxUd\r\n"
	"-----END CERTIFICATE-----\r\n");

QByteArray detach(const QByteArray & ba)
{
	QByteArray rv(ba);
	rv.detach();
	return rv;
}

}
