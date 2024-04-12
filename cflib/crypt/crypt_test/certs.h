/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
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
    "HhcNMjMxMjE1MDgwMDA4WhcNMzMxMjEyMDgwMDA4WjAUMRIwEAYDVQQDDAkxMjcu\r\n"
    "MC4wLjEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCYeKZnrD/nvK5r\r\n"
    "9JFDKKv5oS2XvvoPrmMY4dz7dBE3iMY1gsKcE219h+/NXPLx2E+lus12cvjXuVp/\r\n"
    "JEpQED0kizV2tafxRGW1+c/gMyeMhZPwqfmLhIPupHcF620lMlbbC/zzZNkXwvSf\r\n"
    "LdTraeIHLysaAetQrGNzJGqfW8XAKonxUhphJ67t5t7/qhxVJnSpJd8C2feFWeDC\r\n"
    "ZvUTTFSeeUz36+QGBHcUWcFQXiQmlDlU4Ek6I2E0sRzAlYxBQ2vN2IpqEhPQW9h9\r\n"
    "ybICEaQk5nR1V5x1TWIxi2Ul5fypCD5S3lFzex55IKZO3dTnECJzzbyXmDhKbGsC\r\n"
    "Lbfzzn6tAgMBAAGjezB5MAkGA1UdEwQCMAAwLAYJYIZIAYb4QgENBB8WHU9wZW5T\r\n"
    "U0wgR2VuZXJhdGVkIENlcnRpZmljYXRlMB0GA1UdDgQWBBS+0Xno/ECqe8Q7C6JE\r\n"
    "8rnCMBkQRDAfBgNVHSMEGDAWgBQyoarjQRPJ9NilLLQA5q9Q6wmI9TANBgkqhkiG\r\n"
    "9w0BAQsFAAOCAQEAqEfvlY7TJ+PzSepGJUd04IDEbLlF/Pd0dNH8Ivig7tTAWGYo\r\n"
    "fVs8mWggxHfSnAp80gzEeRz5hGhma1ss4R7hXKXIr8Erx/1iOLCkbYkOtPAe/6IO\r\n"
    "QuvTDjY6DcQraJoM6muBlfw8FF0tG4IyGjQFE6vuoMm5ibGYaSpZYAOa+r9YpfZr\r\n"
    "BAXylgjJD9ZwQNVfx9ot+F3NSnQ2wYx0YE6Qj7mUpGbGbMBWbrFJIenZNvNBKBDu\r\n"
    "IVoiWOhvY7tV7lhnaWKrHAasDn+2e2KZK8tEBaeqhRlRD3PZbllA3uF/KEgxaWot\r\n"
    "AItm3cw+et9t3nBH0CGIUmRRUdkROdS2XBlaqA==\r\n"
    "-----END CERTIFICATE-----\r\n"
);

const QByteArray cert1PrivateKey(
    "-----BEGIN ENCRYPTED PRIVATE KEY-----\r\n"
    "MIIFDjBABgkqhkiG9w0BBQ0wMzAbBgkqhkiG9w0BBQwwDgQIlvd2SuqWuHsCAggA\r\n"
    "MBQGCCqGSIb3DQMHBAip2kUtKfMetQSCBMgaRq37CZ1Tju/d9QDgv71xJ8USFjGM\r\n"
    "1nrXGHPuzflctST27P+jURUar+K4SVrJkabOwebw3MrDTSWOLuqBpP9od6y/X3/u\r\n"
    "lB0TsU20IClwKbv9aE2Db77/oH163OmJWwKbU2MBjod8BDmJW5SrDcV7HgQwRNud\r\n"
    "VTcXXw181gWQcoFiNnH3/vZex0WqJbFv14Sp5d2F02tRTcy1Xm2jH04LdTNuLyAW\r\n"
    "h7Vy2nixW9lQu+q38VHmS9Bn6/6cR5hlVskI2myD/RcwCNQ0W0T0Ugh7GhMLzDFR\r\n"
    "ROCT+dkrX4JMkICz86TE4nc1fJJNunOm0qjFw/t+5J8vOQRqThlwAsTeegeQ2oAw\r\n"
    "3JYuaqpyFadOpamN/eX1/aa6OFjdmpYBckEPbvqIXw6YjkLI3FUMRQ25WIvO23pG\r\n"
    "zpgTO95X2h56PzZIsNdfo6YMzFIfu8RkEfkGKkaRmQBzliWUyRJC9Nta5rn4m6Ky\r\n"
    "a5FYsfWKe9NzDzP//uP07OAAUTMjM9gEOnnZEd6ivkB7GMGQVQ515dCGAyqeOQI7\r\n"
    "LWNvz+rq8WJlxZS6Q3M0ByfexQvb2y+PYw7uCnD39BSr8eFwkKg9n1CvHcsWmjeW\r\n"
    "1kbwKLabl3odPFKB/LLpLnxdMnsWr+8myQ0GJPSoGXrezXweAtHFCUCE9RHC1T/s\r\n"
    "1wZKO6bddWAEYNDZGeXOVvfF3vzctjrsWojS23bI/bHIoPX7cvIRoZxS9/jkVf+n\r\n"
    "f+BYwHyY8E4yfH+VWBmTU9Ucz5RsxBcB74PJlkGAvSzZtOtrEfJNeBm4cAafYQJI\r\n"
    "fYQFCUMAdezXENZ7v5kfLjI4mFyaWxSTcU3qQk+jOqy6ACFloEBkgnjHb+QZEDKM\r\n"
    "cb+ocIhPMUXe3ZX8ehHBlnqFtJb1DogXqeg8LN0sygkP8SXxWGDg//SdPgwZD9uP\r\n"
    "ELWtHJymF93UgfYdg4G+Kn3enwyETQEYLQIjClprAC0PcChlIsrm9LUqNJ0BTpZl\r\n"
    "Jgwo3xGqTQ+GzaTuGflzWjyMWjPKwnWU5VSZG3IUN3kUDIFMh3b4kUv70bk7tqkb\r\n"
    "sT+518VIso/3yGAne/QO7VngANO9kOSf+dfbq5JkZSZnlMT3diocnOGbnV+8xK7v\r\n"
    "GO26c+X1MbHaAXNJSySfKb16Kwdw4bTKdEbir1uiYE6XsMMMkCPcnwcWlxZM9y9d\r\n"
    "HTx87L2iWPbFWToUGtmDnW1tcWKEytNqji95FvksOvoa+P/qO0h4hSbe6MtxUCWh\r\n"
    "o4c0KN1bosQC/Y6/5KuGeNmclD/WMSmkMaQiF1KgkuiyX9YjuaKJ60xZpp7DS+xm\r\n"
    "YVwEIk7/7NqoxU07t8IWTeCmCteMx8SXLccAYZ1m3NoslAOS9bkLoXbSubTNCfws\r\n"
    "6Z1eO/eGdxjfyM7uxecCMYgoFRQvyLPDbrVLqI3AfvYaN3mnDCeIMwy+svNfcMxE\r\n"
    "FbslYOHjWP6XitJuB3v7/UOhwQJbOSxcLRdgeW2E/Y7nmDenxMh+HtZswY236yTq\r\n"
    "lVjyTKBdZNGfS+CR2Uek55rx6zuwQX3SVEyy5/rodhNTu8JOea2/cGoP99qjZscM\r\n"
    "lfv/heoyO5MOKvMh2KT9xgFC9dkJE7U8cjxIcZ4u3xBzULmgdviBXMlo3LIi9rgZ\r\n"
    "wEg=\r\n"
    "-----END ENCRYPTED PRIVATE KEY-----\r\n"
);

const QByteArray cert2(
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIC6jCCAdKgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwETEPMA0GA1UEAwwGcm9v\r\n"
    "dGNhMB4XDTIzMTIxNTA4MDAwOFoXDTMzMTIxMjA4MDAwOFowDTELMAkGA1UEAwwC\r\n"
    "Y2EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCs6ZyNDHlxg7FHUUPA\r\n"
    "QKLn9l8QNnk4uh7MpomRm27QtyFoSgbT1zn++YMBL1jYUAnIgHlzw2HEy4cVXnac\r\n"
    "bC46rEyOKOEMWfj7CKHLk98WA0cWvNH3qChn510I0DfTjxrAhK8luqlsezHCV0W4\r\n"
    "5fu3/iVb5QFrOeY1uT2/DAWXM05ng9drl8zYL8qB7EOZHeazZiY0ZakPf/ns8pR1\r\n"
    "QbLvUboCSW+QYQUdqNV2m+vq+++uQUb11QXO/cW/eAIovmmDDh3nFOGZEWrW3Dc4\r\n"
    "hps6/jsM6OOn44fmJvdfby4TG3xOgiuAk6aZjBLR1BYDK7HCpx7CikmWVXyXptUv\r\n"
    "O3KzAgMBAAGjUDBOMB0GA1UdDgQWBBQyoarjQRPJ9NilLLQA5q9Q6wmI9TAfBgNV\r\n"
    "HSMEGDAWgBQEClF7nIB2p5ClZ8lwQGwsBta8QTAMBgNVHRMEBTADAQH/MA0GCSqG\r\n"
    "SIb3DQEBCwUAA4IBAQDYSzZ82wlawnOqFL9uwsHE1oZHxrzxXsVp6M4S8PvsLMRt\r\n"
    "UxycprVH5hNGLvs85LczzLdsHjHYa7OJ+e0AwrdU1Zrq4fUv9it4UMxVOddwS8yq\r\n"
    "KBZRoiR2BaIvdS+LOadmrHxvmG5ELl1McS+zOCblU1fKcBuqleMKnn+ehRN+DK54\r\n"
    "DkOLy2PCHoRsZvqzoOMa5KbJqLlJKJhUqy1MDe4k/QhCkMxAcNdwmQdGr19qsB7v\r\n"
    "Oyomw7SSJSwf91gk2BmoCeogIF2EeUbqo2J1ZWECbCen5jNGq/MqJFWP487pNete\r\n"
    "ntGX/sk/a1pbLSKmmo/4M/PKuhacPKxGrXsJweCh\r\n"
    "-----END CERTIFICATE-----\r\n"
);

const QByteArray cert2Crl(
    "-----BEGIN X509 CRL-----\r\n"
    "MIIBZTBPAgEBMA0GCSqGSIb3DQEBCwUAMA0xCzAJBgNVBAMMAmNhFw0yMzEyMTUw\r\n"
    "ODAwMDhaFw0zMzEyMTIwODAwMDhaoA4wDDAKBgNVHRQEAwIBATANBgkqhkiG9w0B\r\n"
    "AQsFAAOCAQEAn6yO9JY/d6rR4shteEXvaZIQvs6fRSLE0aDfZojjpv03GDtDejTt\r\n"
    "2uRAv7KgXHnAU4+Xb7S5l/nPGeroe5HCfMkHjoX/6zGVBvjY6UlWZO/ogBsDYKsu\r\n"
    "JqBRf5HUefHnhTN00xkRMfMaz6Fjq0SyYFdPkMwWWc0xXld/N87fmJrQrEF0PHXN\r\n"
    "c3SZfgo4fNph4VRm/HaZFOg4KGEOZVlkQsDmm25G2nVuBGGakpHBkb1ZGaCPOHw+\r\n"
    "psQETzu0eIkOKrzyZEQzkdBZzBctoVhDUGHjGGu1O9vLJv9qxv2m/HVbjX6T1xji\r\n"
    "F4B/LgT/bxaL2mP9S3DaQW+hfDxbGqz+cA==\r\n"
    "-----END X509 CRL-----\r\n"
);

const QByteArray cert3(
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIC9TCCAd2gAwIBAgIJANRBuF+II79zMA0GCSqGSIb3DQEBCwUAMBExDzANBgNV\r\n"
    "BAMMBnJvb3RjYTAeFw0yMzEyMTUwODAwMDhaFw0zMzEyMTIwODAwMDhaMBExDzAN\r\n"
    "BgNVBAMMBnJvb3RjYTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOPD\r\n"
    "Qa+NSH3HDLaUst+QvRMOzt/7m/0wBgXqEvytIRUsuT8rTnKPmrykxVabCkCTq+7F\r\n"
    "rJMAaw7ZuwoqZR3DWWd3CwAaRcR2YHJJWu/vMUipE82xa5k75Cb6F1RtOsTMV+02\r\n"
    "FXb/77x7t/yIKOIxIStWRMwJh58FStJAA2MxuhhIV937HcZzuWH4F0/mP729xEp8\r\n"
    "QYa4cGOVhonM8NhHEoPGsps3lMbzQhiQ3V5nrTVwfQ+Pg8UeQr7kaq+I/5g+JpOL\r\n"
    "I6VX9x2OcNULZkw2cZTODfHwHvXbVRoT7bpfcaZukp/TfkB8q3UBKd2YxkUI38Oq\r\n"
    "ie64MU/BBceQH4wdnrsCAwEAAaNQME4wHQYDVR0OBBYEFAQKUXucgHankKVnyXBA\r\n"
    "bCwG1rxBMB8GA1UdIwQYMBaAFAQKUXucgHankKVnyXBAbCwG1rxBMAwGA1UdEwQF\r\n"
    "MAMBAf8wDQYJKoZIhvcNAQELBQADggEBAHlMLC3fQk1iGYGwTBSyf9hZf+EJ3fpV\r\n"
    "2ZeLe1m+jl2BTWqKIkS0PVCCAiqV/UfJMgTbK2WCBHjRm72D2gq9JFe6YkfB/+qQ\r\n"
    "nyHewnCpuwZNtTebv37+QdlVl5mPgeh/s2C3HbcGyNsDcDugD4GIgV7w376Irl+V\r\n"
    "jcN07NjRTyS7JwbG+llefaiQlKU9ymXj7HP0lr+Uv4bM7rioedE2sKdSBVn5HCd1\r\n"
    "FJngL0i4Z+Cy1DwTEcj1H/G/tiVc+OU1jcT8HJefW4VXTr9CxPfyFzFZdLJG4NWP\r\n"
    "SHk4Vg4kxntDgzAwBYy0dM6iCRZLWfm2AYtQatqoH44houGj8E47nKU=\r\n"
    "-----END CERTIFICATE-----\r\n"
);

inline QByteArray detach(const QByteArray & ba)
{
    QByteArray rv(ba);
    rv.detach();
    return rv;
}

}
