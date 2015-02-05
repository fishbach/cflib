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

#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/util.h>
#include <cflib/util/test.h>

using namespace cflib::crypt;

class TLSCredentials_test : public QObject
{
	Q_OBJECT
private slots:

	void initTestCase()
	{
		QVERIFY(initCrypto());
	}

	void test_load()
	{
		TLSCredentials creds;
		QCOMPARE((int)creds.addCert(QByteArray()), 0);
		QCOMPARE((int)creds.addCert(
			"-----BEGIN CERTIFICATE-----\r\n"
			"MIIFPjCCBCagAwIBAgIDEjoRMA0GCSqGSIb3DQEBBQUAMDwxCzAJBgNVBAYTAlVT\r\n"
			"MRcwFQYDVQQKEw5HZW9UcnVzdCwgSW5jLjEUMBIGA1UEAxMLUmFwaWRTU0wgQ0Ew\r\n"
			"HhcNMTQwNDE2MDYzMTEyWhcNMTUwNDE5MDg0NTA5WjCByzEpMCcGA1UEBRMgNXA4\r\n"
			"eXV4VjgyS3F6c2xWajZSUkRoWFRJNkpHMVpleTExEzARBgNVBAsTCkdUMjgzMDYy\r\n"
			"NTMxMTAvBgNVBAsTKFNlZSB3d3cucmFwaWRzc2wuY29tL3Jlc291cmNlcy9jcHMg\r\n"
			"KGMpMTQxLzAtBgNVBAsTJkRvbWFpbiBDb250cm9sIFZhbGlkYXRlZCAtIFJhcGlk\r\n"
			"U1NMKFIpMSUwIwYDVQQDExxyZWdpc3RyYXRpb24uc3VtbWVyY291cnNlLmVjMIIB\r\n"
			"IjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzal8DoVvjJMvmcJZhoMrtOGr\r\n"
			"kgx7zlaPvx3sNbZQJGmcwEZoOWtgw/plVn/k8dh9my1h76EnLOHkxYaT+S9brUzH\r\n"
			"uCtM2JQC5Mn6tJC13aOcJDl96Wb2ha1GAiXpgYa0fSoLDOFdT4nbjCXvz4mADOz6\r\n"
			"6PblBR/teV3HXTUyvRSYpTDA/oUIid/u/TDd2fN+QflxoiFu2N7GUQI04MgfqGgQ\r\n"
			"LTjWSSltbnDefuCgjKaDFU/cXL0pPShoPNcADqVWKIfiFDd3mKY/NvTB+HfmUuGF\r\n"
			"jYBFhoX+nbhHSrsvZEpYJp5RZplPis4jaxz17D6dXcyN4jTq087oEEuNx8NVjQID\r\n"
			"AQABo4IBtzCCAbMwHwYDVR0jBBgwFoAUa2k9ahhCSt2PAmU5/TUkhniRFjAwDgYD\r\n"
			"VR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAnBgNV\r\n"
			"HREEIDAeghxyZWdpc3RyYXRpb24uc3VtbWVyY291cnNlLmVjMEMGA1UdHwQ8MDow\r\n"
			"OKA2oDSGMmh0dHA6Ly9yYXBpZHNzbC1jcmwuZ2VvdHJ1c3QuY29tL2NybHMvcmFw\r\n"
			"aWRzc2wuY3JsMB0GA1UdDgQWBBR6WYfxkgQBoZVuZX529DKHr5BnLjAMBgNVHRMB\r\n"
			"Af8EAjAAMHgGCCsGAQUFBwEBBGwwajAtBggrBgEFBQcwAYYhaHR0cDovL3JhcGlk\r\n"
			"c3NsLW9jc3AuZ2VvdHJ1c3QuY29tMDkGCCsGAQUFBzAChi1odHRwOi8vcmFwaWRz\r\n"
			"c2wtYWlhLmdlb3RydXN0LmNvbS9yYXBpZHNzbC5jcnQwTAYDVR0gBEUwQzBBBgpg\r\n"
			"hkgBhvhFAQc2MDMwMQYIKwYBBQUHAgEWJWh0dHA6Ly93d3cuZ2VvdHJ1c3QuY29t\r\n"
			"L3Jlc291cmNlcy9jcHMwDQYJKoZIhvcNAQEFBQADggEBAJ9PPWPfQprZHrHpK+LA\r\n"
			"+qz0GXCbulhHYVdcSWctJ19KZr4OKF+dYZVYR8HfnfX24weRpuLw47a6BggK1JmD\r\n"
			"IrvPlmf0l/hKZ0FETmYI7h4GT1/eHD3izy1vfj8ecun/Okyt73IaM4aUSH700tLS\r\n"
			"IxGahXvX5mJ3jf1bMQvMPYuX1oMRPVfVC5cGEet+XAhofGPh8gz4LtYT/X8LNeiI\r\n"
			"WkvakECVzTfvpwjEINDwU4vhNBxmET5dC2MXjGpTW+QyBhF5Tl+8IehNK+DqphPp\r\n"
			"5wLZW/LnWXuucEqEbmRH6xtBWftq4qf2hJujTPmotJhoIf9SuXDu1FBstOhJHhVW\r\n"
			"aTk=\r\n"
			"-----END CERTIFICATE-----\r\n"
		), 1);
		QCOMPARE((int)creds.addCert(
			"-----BEGIN CERTIFICATE-----\r\n"
			"MIID1TCCAr2gAwIBAgIDAjbRMA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT\r\n"
			"MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i\r\n"
			"YWwgQ0EwHhcNMTAwMjE5MjI0NTA1WhcNMjAwMjE4MjI0NTA1WjA8MQswCQYDVQQG\r\n"
			"EwJVUzEXMBUGA1UEChMOR2VvVHJ1c3QsIEluYy4xFDASBgNVBAMTC1JhcGlkU1NM\r\n"
			"IENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAx3H4Vsce2cy1rfa0\r\n"
			"l6P7oeYLUF9QqjraD/w9KSRDxhApwfxVQHLuverfn7ZB9EhLyG7+T1cSi1v6kt1e\r\n"
			"6K3z8Buxe037z/3R5fjj3Of1c3/fAUnPjFbBvTfjW761T4uL8NpPx+PdVUdp3/Jb\r\n"
			"ewdPPeWsIcHIHXro5/YPoar1b96oZU8QiZwD84l6pV4BcjPtqelaHnnzh8jfyMX8\r\n"
			"N8iamte4dsywPuf95lTq319SQXhZV63xEtZ/vNWfcNMFbPqjfWdY3SZiHTGSDHl5\r\n"
			"HI7PynvBZq+odEj7joLCniyZXHstXZu8W1eefDp6E63yoxhbK1kPzVw662gzxigd\r\n"
			"gtFQiwIDAQABo4HZMIHWMA4GA1UdDwEB/wQEAwIBBjAdBgNVHQ4EFgQUa2k9ahhC\r\n"
			"St2PAmU5/TUkhniRFjAwHwYDVR0jBBgwFoAUwHqYaI2J+6sFZAwRfap9ZbjKzE4w\r\n"
			"EgYDVR0TAQH/BAgwBgEB/wIBADA6BgNVHR8EMzAxMC+gLaArhilodHRwOi8vY3Js\r\n"
			"Lmdlb3RydXN0LmNvbS9jcmxzL2d0Z2xvYmFsLmNybDA0BggrBgEFBQcBAQQoMCYw\r\n"
			"JAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmdlb3RydXN0LmNvbTANBgkqhkiG9w0B\r\n"
			"AQUFAAOCAQEAq7y8Cl0YlOPBscOoTFXWvrSY8e48HM3P8yQkXJYDJ1j8Nq6iL4/x\r\n"
			"/torAsMzvcjdSCIrYA+lAxD9d/jQ7ZZnT/3qRyBwVNypDFV+4ZYlitm12ldKvo2O\r\n"
			"SUNjpWxOJ4cl61tt/qJ/OCjgNqutOaWlYsS3XFgsql0BYKZiZ6PAx2Ij9OdsRu61\r\n"
			"04BqIhPSLT90T+qvjF+0OJzbrs6vhB6m9jRRWXnT43XcvNfzc9+S7NIgWW+c+5X4\r\n"
			"knYYCnwPLKbK3opie9jzzl9ovY8+wXS7FXI6FoOpC+ZNmZzYV+yoAVHHb1c0XqtK\r\n"
			"LEL2TxyJeN4mTvVvk0wVaydWTQBUbHq3tw==\r\n"
			"-----END CERTIFICATE-----\r\n"
			"-----BEGIN CERTIFICATE-----\r\n"
			"MIIDVDCCAjygAwIBAgIDAjRWMA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT\r\n"
			"MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i\r\n"
			"YWwgQ0EwHhcNMDIwNTIxMDQwMDAwWhcNMjIwNTIxMDQwMDAwWjBCMQswCQYDVQQG\r\n"
			"EwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMSR2VvVHJ1c3Qg\r\n"
			"R2xvYmFsIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2swYYzD9\r\n"
			"9BcjGlZ+W988bDjkcbd4kdS8odhM+KhDtgPpTSEHCIjaWC9mOSm9BXiLnTjoBbdq\r\n"
			"fnGk5sRgprDvgOSJKA+eJdbtg/OtppHHmMlCGDUUna2YRpIuT8rxh0PBFpVXLVDv\r\n"
			"iS2Aelet8u5fa9IAjbkU+BQVNdnARqN7csiRv8lVK83Qlz6cJmTM386DGXHKTubU\r\n"
			"1XupGc1V3sjs0l44U+VcT4wt/lAjNvxm5suOpDkZALeVAjmRCw7+OC7RHQWa9k0+\r\n"
			"bw8HHa8sHo9gOeL6NlMTOdReJivbPagUvTLrGAMoUgRx5aszPeE4uwc2hGKceeoW\r\n"
			"MPRfwCvocWvk+QIDAQABo1MwUTAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTA\r\n"
			"ephojYn7qwVkDBF9qn1luMrMTjAfBgNVHSMEGDAWgBTAephojYn7qwVkDBF9qn1l\r\n"
			"uMrMTjANBgkqhkiG9w0BAQUFAAOCAQEANeMpauUvXVSOKVCUn5kaFOSPeCpilKIn\r\n"
			"Z57QzxpeR+nBsqTP3UEaBU6bS+5Kb1VSsyShNwrrZHYqLizz/Tt1kL/6cdjHPTfS\r\n"
			"tQWVYrmm3ok9Nns4d0iXrKYgjy6myQzCsplFAMfOEVEiIuCl6rYVSAlk6l5PdPcF\r\n"
			"PseKUgzbFbS9bZvlxrFUaKnjaZC2mqUPuLk/IH2uSrW4nOQdtqvmlKXBx4Ot2/Un\r\n"
			"hw4EbNX/3aBd7YdStysVAq45pmp06drE57xNNB6pXE0zX5IJL4hmXXeXxx12E6nV\r\n"
			"5fEWCRE11azbJHFwLJhWC9kXtNHjUStedejV0NxPNO3CBWaAocvmMw==\r\n"
			"-----END CERTIFICATE-----\r\n"
		), 2);
	}

};
#include "tlscredentials_test.moc"
ADD_TEST(TLSCredentials_test)
