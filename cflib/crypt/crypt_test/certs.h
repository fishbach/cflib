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
openssl req -new -key server_key.pem -out server_csr.pem
openssl ca -policy policy_anything -keyfile ca_key.pem -cert ca_crt.pem -in server_csr.pem -out server_crt.pem -days 3650
openssl pkcs8 -nocrypt -topk8 -in server_key.pem

*/

const QByteArray cert1(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDVDCCAjygAwIBAgICEAEwDQYJKoZIhvcNAQEFBQAwSTELMAkGA1UEBhMCREUx\r\n"
	"CzAJBgNVBAgTAkRFMRMwEQYDVQQKEwpDQSBDb21wYW55MRgwFgYDVQQDEw9JbnRl\r\n"
	"cm1lZGlhdGUgQ0EwHhcNMTUwMjExMDk1MjI5WhcNMjUwMjA4MDk1MjI5WjAUMRIw\r\n"
	"EAYDVQQDEwlzZXJ2ZXIubXkwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIB\r\n"
	"AQD3W3bjp9pheF3mx9lRuwmaSgN3dkJWGkh+yaAmCB4nIEJH/+lHbiVIk+0Mp7u7\r\n"
	"Mtw7UDzrNdGHz8lwfrqEm8uHRPv+YiUt5V2EXdjrjv2Ol/DBlyZb6j24F5U9J/8r\r\n"
	"Sx6dG4Gk/q0iEpdaLklsQZ3nC+dT3KhUpr/2pBW88gJlx4yo0upLvPzSTsfPPi3o\r\n"
	"VJMyJO7ZIfKya7PJATmyG3b8eox//9ehhAlWYyZU6wU4H2ADc580bDVaaKBFZ88o\r\n"
	"OZ96XJHpBCwZ039Mv5klSxIEPrTL7Zam7pOhvcNkfhEWyx0VmCI8BFBOTfDSf8dK\r\n"
	"PUma7oRWuwkKPc23+hA2e20VAgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJYIZIAYb4\r\n"
	"QgENBB8WHU9wZW5TU0wgR2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1UdDgQWBBQk\r\n"
	"6LfSvr7WboZhKx/yMzvp1ph3SDAfBgNVHSMEGDAWgBSK0RGD4rwPCd6O0AZU3/zf\r\n"
	"ikmnLTANBgkqhkiG9w0BAQUFAAOCAQEAi7ebyoc6+HC4gZSTXZ2qDAelTZgv1ojg\r\n"
	"pjUmy8E2y0Q5+DXf/gS8vLy/JY/1Z9yaK+0I1LsjvgHxC4XT/Y8qHyDzNaLPgu8I\r\n"
	"hHnDNsfpO+qqMQzv1CVST0DXVGQhWpshIo9cpGbPTEPrWPPZHhvpyzzWp+EJqs6E\r\n"
	"xe6A3PsDxZTEg6RzLhGjFlH1Rveg8mqBy4+y6Q4fPjHlMEipI0QPR+MLyQVY/Dpx\r\n"
	"NylDJokC06J5eUIDRYlkHNwqPKhmXSd3KYOfWSQrjn80ubupnPYTZWLdTaUy2MNy\r\n"
	"CHdNH30CrDnNYA2utInSHc2YwM256tXcezs6YBv78SpRBjO6Z+zA6Q==\r\n"
	"-----END CERTIFICATE-----\r\n");
const QByteArray cert2(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDKTCCApKgAwIBAgICEAAwDQYJKoZIhvcNAQEFBQAwQTELMAkGA1UEBhMCREUx\r\n"
	"CzAJBgNVBAgTAkRFMRMwEQYDVQQKEwpDQSBDb21wYW55MRAwDgYDVQQDEwdSb290\r\n"
	"IENBMB4XDTE1MDIxMTA5NTE0OFoXDTI1MDIwODA5NTE0OFowSTELMAkGA1UEBhMC\r\n"
	"REUxCzAJBgNVBAgTAkRFMRMwEQYDVQQKEwpDQSBDb21wYW55MRgwFgYDVQQDEw9J\r\n"
	"bnRlcm1lZGlhdGUgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCa\r\n"
	"tad5LyGLISwBe2IhXNSg1cA/vezxlgtQuDWJkxEolymUOgwVSFB6G8Fjf/kxZwfd\r\n"
	"++BGi9wny/Os2QVFG1i/rkIwBbpyJYuYg19qTo4i9MryfJ3dxPIG22aedzsnv1wn\r\n"
	"mEUiRN3UYhIBxvvtudV+skMJH5uPpRXfEjLT7vns0zNB/2N7AjORVR9RcQJgEJWA\r\n"
	"0t/wwOUa1av9dLVwh+YxDYIfRvyx7I743Ax+oWsaZJX0Vvvuo1CVX9EYMBZNufJm\r\n"
	"G0EQlGP/rKY+WOeXTqfw3D3OqjqBvgxy31y1o7M5L+hGHl+rjX6EhkcTHlTPasF1\r\n"
	"LbBfojjv0T1ricPTc9PvAgMBAAGjgaMwgaAwHQYDVR0OBBYEFIrREYPivA8J3o7Q\r\n"
	"BlTf/N+KSactMHEGA1UdIwRqMGiAFBpV3Wy9ShfSY9DlbKUoovFORFvhoUWkQzBB\r\n"
	"MQswCQYDVQQGEwJERTELMAkGA1UECBMCREUxEzARBgNVBAoTCkNBIENvbXBhbnkx\r\n"
	"EDAOBgNVBAMTB1Jvb3QgQ0GCCQDs7MMxajcg8zAMBgNVHRMEBTADAQH/MA0GCSqG\r\n"
	"SIb3DQEBBQUAA4GBAAz1TqXcTg7E0LEWIEHf2Wy+bPXQoazFT3yzFJvBWyzNaBTK\r\n"
	"j6RL7DMZyXF9N0ISofUpJmQ6bhttOXOWUIl78CFGYjguMmWDe4yH0o3eF7QudHyn\r\n"
	"uD3d921nh4xXCcWyL0FFaEe7r2hOJ3C3rSXa5y0rhjzxKUodyNbMV9i4QANw\r\n"
	"-----END CERTIFICATE-----\r\n");
const QByteArray cert3(
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIICpDCCAg2gAwIBAgIJAOzswzFqNyDzMA0GCSqGSIb3DQEBBQUAMEExCzAJBgNV\r\n"
	"BAYTAkRFMQswCQYDVQQIEwJERTETMBEGA1UEChMKQ0EgQ29tcGFueTEQMA4GA1UE\r\n"
	"AxMHUm9vdCBDQTAeFw0xNTAyMTEwOTUwNThaFw0yNTAyMDgwOTUwNThaMEExCzAJ\r\n"
	"BgNVBAYTAkRFMQswCQYDVQQIEwJERTETMBEGA1UEChMKQ0EgQ29tcGFueTEQMA4G\r\n"
	"A1UEAxMHUm9vdCBDQTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAs1DCmUs/\r\n"
	"l6ZtPoiWlM3zjcvo5PTKLuefQBTdDlehVxfHUP+A1b0D1P1XWrPm2Hv2S221vRiK\r\n"
	"A2N+t0uaMdNreQVjatXCDGAJY34PmTsLQFxPicc0WzDsmexQFsV0Oxzq/GwULuJS\r\n"
	"s4WYOKSR2JRDukcZT9wAfLJ9HIDszXKIIFcCAwEAAaOBozCBoDAdBgNVHQ4EFgQU\r\n"
	"GlXdbL1KF9Jj0OVspSii8U5EW+EwcQYDVR0jBGowaIAUGlXdbL1KF9Jj0OVspSii\r\n"
	"8U5EW+GhRaRDMEExCzAJBgNVBAYTAkRFMQswCQYDVQQIEwJERTETMBEGA1UEChMK\r\n"
	"Q0EgQ29tcGFueTEQMA4GA1UEAxMHUm9vdCBDQYIJAOzswzFqNyDzMAwGA1UdEwQF\r\n"
	"MAMBAf8wDQYJKoZIhvcNAQEFBQADgYEAALPE3tTwKdGIXj+DrFwscB3jVLLgHaxz\r\n"
	"ZlYJcf09wDqQdRbcfZISWVzlDafQA8/G7aDOnDH9txBLe5/OauMKYKimgi17FJjb\r\n"
	"LUWW2dx+rqulVuhFYBqAS7v2aqrQgLoaKxHD0o0JsHMytxJzxcP3SVHKKsI2+pkE\r\n"
	"/Yul8yRqDAc=\r\n"
	"-----END CERTIFICATE-----\r\n");
const QByteArray cert1PrivateKey(
	"-----BEGIN PRIVATE KEY-----\r\n"
	"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQD3W3bjp9pheF3m\r\n"
	"x9lRuwmaSgN3dkJWGkh+yaAmCB4nIEJH/+lHbiVIk+0Mp7u7Mtw7UDzrNdGHz8lw\r\n"
	"frqEm8uHRPv+YiUt5V2EXdjrjv2Ol/DBlyZb6j24F5U9J/8rSx6dG4Gk/q0iEpda\r\n"
	"LklsQZ3nC+dT3KhUpr/2pBW88gJlx4yo0upLvPzSTsfPPi3oVJMyJO7ZIfKya7PJ\r\n"
	"ATmyG3b8eox//9ehhAlWYyZU6wU4H2ADc580bDVaaKBFZ88oOZ96XJHpBCwZ039M\r\n"
	"v5klSxIEPrTL7Zam7pOhvcNkfhEWyx0VmCI8BFBOTfDSf8dKPUma7oRWuwkKPc23\r\n"
	"+hA2e20VAgMBAAECggEBAOX/zBp+1xZOn7GZHj9a1OD79SE9ew0ov0P1Is56Od9T\r\n"
	"pY3hLP5YMp67vW1JAcxczF7yitKCZmQMF1hc3f20B5mt0UqF4+A7J6uOv8a4XJgc\r\n"
	"O6cmbmqE8gKdGw2UhTWyUbLwAqpyjHXkJ4uZAniAwtw1O1HFGVgs/M3PYM8hkI0l\r\n"
	"HWoiZGcs2DNT9JacKzuqg8BoE9NIhXgN2zcJ1rto35p9LpzTccB3i5jifx2I+4cE\r\n"
	"FQUKHd4adEZQtp1NtLyW7b1CWzjcsx0pg/8EWWRPK8grnBo15iGHuHgT6OiR91d9\r\n"
	"oj6ZM2cP55HvMrKsVXy+0oPnnl6ZS2PV/GfczNOX8AECgYEA/pjR6xA8+L3xW1/x\r\n"
	"VVrKAImiO4M5a3dD+AkzG8nRfzo4p60Mrbd+Rpi5loUxjXVhiamhYT0KRJTrSzA3\r\n"
	"jEInmnhvi0RJymXEQpIxwSvyHfJ/YkyA6Ry16xPCdyPSbssqT7pJK5hGDG8u48+d\r\n"
	"yCKDUW4j+8nobq8hoR4sx0fYBxUCgYEA+LhuS89WCtE1DWoUUWqF3KAb+hdjQLpF\r\n"
	"ic3ZTQMVotFEuH7A289ILZhw8NiFXNc8UhLg6HmpkNnm+ZfwVsBYzgyjVIQt2aaQ\r\n"
	"M+mt6YxWeXQcpnwWxCTGwtqJ9ZnYbOC+Dxno5WoZok9+nMC/GBzpayUG825IamSD\r\n"
	"tflNWnlPTgECgYBqJ9IvTv/9P0WpseFwk4BypPCuG9MPShVfEIbs6UOe5unEkFUf\r\n"
	"Acl3KisH5dV5hB39Rmtxnf/wBJ/vI9Wld9gHgnwP95NE/xXMJCT2xJoZfok6tWdy\r\n"
	"y3PW5tBI2PjfFXs47xWfRci/WoSUnHbPggR7KY97Zv6xV/iPs4M8PGqI2QKBgDnl\r\n"
	"9QcAxO7PtKHix8gMHA032EtnlltDAV0K5kLfLPIx9OVd3FdO8WZrzh7CyIjMMeaJ\r\n"
	"LoHvkGF/2BzSGciYDhQpLPxHqMS52We8RhfP63FqbiDzUflm9j310ZeysIbn6lZp\r\n"
	"maWOBVmbV8X1uIZMdKnUfMG+Mm1R6Lc7yZ3+D1ABAoGASjBAnFXMEwPadYLMXTaE\r\n"
	"gnFkQR8taxh9LmOEEdHkoRDwEchtVHXU8s2wsAgMtbq/I3q9NBp0p95crc/DRtMH\r\n"
	"QQ5QFJNcq7HUVSsP/OXP8IU0xItQc+snwJNKkyN7Go7EMsaryctq28ENea5Ty1YF\r\n"
	"hzTHYJWA5a8DIrBBJ9+9z9U=\r\n"
	"-----END PRIVATE KEY-----\r\n");

}
