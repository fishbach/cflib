/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace {

const QByteArray cert1(
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDGDCCAgCgAwIBAgICEAEwDQYJKoZIhvcNAQELBQAwDTELMAkGA1UEAwwCY2Ew\r\n"
    "HhcNMjMwOTE4MDcxNjE3WhcNMzMwOTE1MDcxNjE3WjAUMRIwEAYDVQQDDAkxMjcu\r\n"
    "MC4wLjEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDqPNOPItFzCXPx\r\n"
    "6+pmq8UAZ/fTSWLAb3YTyU0ZbbY53DDAvTm5fk621pJFFW1nM52yizXK9gw6jpJ0\r\n"
    "Km0EK9eAfayzqDbsbVUbNmPsPAZHCVpFa7Lr7cNRAdGVX7MNgKq5c6D7WXvS8ZbK\r\n"
    "AZ+BxWG/RFoC89kD/NORMaKJi/ZrK7zMct15QXUFRgqefbM1SWulZ9CWJwx2ONh1\r\n"
    "YOg/ceVAlYh381UdgOyYyThAfK8BjZ79HalrruYeq1CVZLmuhQwxvYwWnDNviCnh\r\n"
    "EqmTjk7XQXzebNFiOjfV2mvJGt633aHIojL/salvwulSdyVTYNFhgGALENwbn36O\r\n"
    "tUaSvfgDAgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJYIZIAYb4QgENBB8WHU9wZW5T\r\n"
    "U0wgR2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1UdDgQWBBQ3rKZmeNEI6aAjSIBW\r\n"
    "Rk44czL2xTAfBgNVHSMEGDAWgBTptAe/vDx34LhuWcHfp66y8KF8mTANBgkqhkiG\r\n"
    "9w0BAQsFAAOCAQEAiO5vKnvjHuat9tcROY6Kr0EqQfjCT7EtfVNRmikvY87ZGE63\r\n"
    "MNXo/0JyYc/FCEZbiPSUCxvTpNCdo+qS6CTuAu4LN1Cr1wtoqSMsFn9ELAyFs96r\r\n"
    "UUIfRk/vScZ2qKXl6ZpJDSmp32hAsfeWgtYIH1TruiYLF+J60b60uQOhZmDsYD5C\r\n"
    "jGL3xclF+cVfNTCHURsjYX6Qxt+2DhYHlr35LfvCzoFwQdzTuRS0/llxn6ekQ3Kl\r\n"
    "jVUWAHT4PfZfpBLyjYdVgjGYPUinp2nQDNRp1XNNl0vGRmsaRitCJGkMyWhKJmsU\r\n"
    "yRHyZvArjCcOQjG6MeRUCWXzBcnOLrV4Y5oweQ==\r\n"
    "-----END CERTIFICATE-----\r\n"
);

const QByteArray cert1PrivateKey(
    "-----BEGIN ENCRYPTED PRIVATE KEY-----\r\n"
    "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\r\n"
    "9zo=\r\n"
    "-----END ENCRYPTED PRIVATE KEY-----\r\n"
);

const QByteArray cert2(
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIC6jCCAdKgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwETEPMA0GA1UEAwwGcm9v\r\n"
    "dGNhMB4XDTIzMDkxODA3MTYxN1oXDTMzMDkxNTA3MTYxN1owDTELMAkGA1UEAwwC\r\n"
    "Y2EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCurixoel1XJ5ltKlqs\r\n"
    "53IxIj2xpYfwaSv0GFWUlf4jGmSxh2vBxr/FzuwpB9lvxOWB7RXIEUhjOnqYdczQ\r\n"
    "MFpYkPYP0M4z4CDGVCnKVP2LirO+pq1jIjoakI88i8YLyey/8aykIjiGhsGTU8Sq\r\n"
    "+NLJSjgGLy8GyE8Oy3lYQV1sV0y8tDpaihcSuEoiUD62GxHYG1wiuDN3bZfEtmsI\r\n"
    "tTzyXJhwZQb0oOA0E7JFAObMU11muUHNT96ZZFMYogrcyaVn3bmh9lYe+4WXVX+8\r\n"
    "o67Sfe48y6gPAusXDrxKxwPyHJfADKrKlBVbOciaFU1++RouaE3l5m9Zophrxt8g\r\n"
    "vdTDAgMBAAGjUDBOMB0GA1UdDgQWBBTptAe/vDx34LhuWcHfp66y8KF8mTAfBgNV\r\n"
    "HSMEGDAWgBQI1yECaMiUSa5hklV+QoF4IOsD9TAMBgNVHRMEBTADAQH/MA0GCSqG\r\n"
    "SIb3DQEBCwUAA4IBAQCe/saxiuo936ev9JCDbIA3LedDBhSY8BC4w1udUCvYO5oV\r\n"
    "TZSccVf+lQRlMqIE5qEl72cIPv5Nf5qsP3AHYfbyJYGCEDOkpxE2/t+hOHQttKQw\r\n"
    "UfD4YKZF0r2Ns3FNThdOAUpXaswUpe99h5TfoJvn2+sfrzId9qAwR2edvnFnKqLL\r\n"
    "ZO0dN8lMfTKduxZu2udRgr/gryW5RpFFHfyCMnDpZsfwE3rq5lJIWouillapLZ2A\r\n"
    "+fu5DkieOP/ReKd84z9/JeWG8Y35MIYGXW9bD5txxI9sSolKyIbv01KkxcNMSEVB\r\n"
    "Hkl4b2hOQJmcwowjmb3QogOAZ7Iimw9tNIHzw1qm\r\n"
    "-----END CERTIFICATE-----\r\n"
);

const QByteArray cert2Crl(
    "-----BEGIN X509 CRL-----\r\n"
    "MIIBZTBPAgEBMA0GCSqGSIb3DQEBCwUAMA0xCzAJBgNVBAMMAmNhFw0yMzA5MTgw\r\n"
    "NzE2MTdaFw0zMzA5MTUwNzE2MTdaoA4wDDAKBgNVHRQEAwIBATANBgkqhkiG9w0B\r\n"
    "AQsFAAOCAQEABbmZGvY/lChe3vLz4zxKWm/EumBnEmz/qIX3ZIXl/RgwMr9LObS/\r\n"
    "RUHNPA/8mPQhEnOkQrMx/C7QxRKaxslpXmBCmPB1TqHsy6Zit/kiytaTmM2RzYMZ\r\n"
    "9guofjvWmKoHwBCPmOtHMWc5q2u8TdjbY7oHpawvb2/R2BT99c4nKPteZ+6Tsbb4\r\n"
    "/d2BahnS8FGuP4I5DedIgmS6aHhxrPCV4MsqzfzNT/x+H/vlZvenkVl74LNo49fW\r\n"
    "zgYdGhaag66zDYOqRPgJqyYpKE8TqQQVlQj656+jaUFkukTIOGJk000E4MS0fzDL\r\n"
    "4bDO7ogEpg71jsi41I93f5pdeacWxPwbbg==\r\n"
    "-----END X509 CRL-----\r\n"
);

const QByteArray cert3(
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIC9TCCAd2gAwIBAgIJAMaGhepYwMq6MA0GCSqGSIb3DQEBCwUAMBExDzANBgNV\r\n"
    "BAMMBnJvb3RjYTAeFw0yMzA5MTgwNzE2MTdaFw0zMzA5MTUwNzE2MTdaMBExDzAN\r\n"
    "BgNVBAMMBnJvb3RjYTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOUQ\r\n"
    "vhDMYRYoA6Teigqxj1+kEl2ENUW1dhGwEQmj08V1X/B0SYGKsJsAWze5UWblhi8j\r\n"
    "VXzalOW8rrZqEaiUb6v5xZy89yq3fN3DZDz/lQHQB2aiaBfLzm8ymOU01f9ccqFH\r\n"
    "9xFsXlwDH4+xU4hbIWUY70WoThD3FKcd0oukzJcdUdUVq+Ak6D1BGNYj9j7pF3Jr\r\n"
    "ILgzZY34yOlrGUPnmaBuWohFuadUhzt3hioXdl+q/GHkXpxnJdusKVS+wWjZCNX7\r\n"
    "iflMnYCvsXvr7Iaa+MYkFDj+VljldUQ4Fgta0AOReT3Oq7iKdoL2tY+vwMf/zqdg\r\n"
    "Idk11GdcGtGr/5aI7nsCAwEAAaNQME4wHQYDVR0OBBYEFAjXIQJoyJRJrmGSVX5C\r\n"
    "gXgg6wP1MB8GA1UdIwQYMBaAFAjXIQJoyJRJrmGSVX5CgXgg6wP1MAwGA1UdEwQF\r\n"
    "MAMBAf8wDQYJKoZIhvcNAQELBQADggEBAB8FuUwoS8W3ZivaGLZx5ET6wuoZgNHZ\r\n"
    "G7es8WhN0zCXcbLplk19hB+mm9IBWqbJWmakA9LpXjwTRVjGwVjMcFJ2U7QnGea/\r\n"
    "wJ5x1oHeYZNk2xaSADuw6DAe+DG5TlKQsX1qZOeR+BOPKI83XglQabXDTXaEtkYW\r\n"
    "HtszRyStYBY29w8m+ddEmvXjzL0++lYjGlmqVRFlDK+59SKCTFgHYyaunbOxEa7c\r\n"
    "LsDo4zqcZ28LOz8RLW+C/ERi8sKeIAVfewjiQz7PL4MEiuusqIRuFSnfvtsxER1H\r\n"
    "P/gRZvOOIdyctqRj/1MpteEbrjQ/Q4pwpeOfspt+GyKBshI2ZfXcVIM=\r\n"
    "-----END CERTIFICATE-----\r\n"
);

inline QByteArray detach(const QByteArray & ba)
{
    QByteArray rv(ba);
    rv.detach();
    return rv;
}

}
