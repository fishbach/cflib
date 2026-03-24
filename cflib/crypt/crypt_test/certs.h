/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace {

const ByteArray cert1(
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDKzCCAhOgAwIBAgICEAEwDQYJKoZIhvcNAQELBQAwDTELMAkGA1UEAwwCY2Ew\r\n"
    "HhcNMjYwMTExMTEzMTQ5WhcNMzYwMTA5MTEzMTQ5WjAUMRIwEAYDVQQDDAkxMjcu\r\n"
    "MC4wLjEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCpTZdTTSRjrlBy\r\n"
    "JZIAYJKguGiyaeVk2ybPI+FkelE2tEfRSJlaRVSoZOhYbTvNSuuI4O5AcVe9oKXf\r\n"
    "LmHbX8d0RWC+0PUwNTTgz/j5LvYGkaJ5ddRLrCOLcY7q011wfPMF2LIurItb6wgB\r\n"
    "IiCm4DTAK3t7cJN+jikOF2lV/Kk7+X65xsIpHoSdom6xZ5bWor2M+NoBegNX20IK\r\n"
    "406zWfPppoVj8AB24zxqfQSOEs9Z5mY4V2hBNVBPRUs6uQ9ASdeiXHna8mJluxe9\r\n"
    "Qky4EIewH9Zs08PZdeysA/eXjF1hbdjsW9DgKiGUYPq62GU/F8xbsdWjmImZ5OX0\r\n"
    "ampfqr5fAgMBAAGjgY0wgYowCQYDVR0TBAIwADAsBglghkgBhvhCAQ0EHxYdT3Bl\r\n"
    "blNTTCBHZW5lcmF0ZWQgQ2VydGlmaWNhdGUwHQYDVR0OBBYEFA/Z55ZLmhqNmBNi\r\n"
    "UL2QhQRBK25AMB8GA1UdIwQYMBaAFG0lwwjVZnv9z0AiSmMPUW3y0qrlMA8GA1Ud\r\n"
    "EQQIMAaHBH8AAAEwDQYJKoZIhvcNAQELBQADggEBAFi6lNQpYLQYKdscfnX3QRIg\r\n"
    "mnJzGdxTirp2UjLRGGxE4+qwFbINBs3ptEZrKlt4liXa2/dGRP8mc/2frvbVCeDa\r\n"
    "RMncOxmN4jrDULx8oMqQ7W5qbI1awyNZAzUkxQCRaNzkyQF1gPWnP5RY4u2HuZ88\r\n"
    "ddpPnI2nwkP6eYkyeLe+DRPq95LvFS5kDJEljib3ODdyMn1DnjKFYeTHgk8aDOZl\r\n"
    "ZrseYoE70oGeiRxAvYa0Ui2G+Lmt0H5tyS2N+ZaoS6I7yuf5bYDUFLgwgXykRCgG\r\n"
    "rsU+dVnoOpi+aVayBv0tlt6QeVqVzKMRzJLQ30iamRyo6pyLKGWtjI+K4gTzGp0=\r\n"
    "-----END CERTIFICATE-----\r\n"
);

const ByteArray cert1PrivateKey(
    "-----BEGIN ENCRYPTED PRIVATE KEY-----\r\n"
    "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\r\n"
    "HaEw8qKRzxLx4EAPL3TOriBmjMg7PNrm\r\n"
    "-----END ENCRYPTED PRIVATE KEY-----\r\n"
);

const ByteArray cert2(
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIC6jCCAdKgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwETEPMA0GA1UEAwwGcm9v\r\n"
    "dGNhMB4XDTI2MDExMTExMzE0OVoXDTM2MDEwOTExMzE0OVowDTELMAkGA1UEAwwC\r\n"
    "Y2EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDVfVTtpwSJJHXapj6H\r\n"
    "WX+b9h1ldgksz+WASEteEZfTgbkKmD2us/GPPRBIYjaJTr3shTyl45nlBRb2IaVu\r\n"
    "abtaUiQASK/+w9tttHBSz/htVQixZ9nqxlvQAFOhGv5p8SZr7uPHNei+rdgi3ikf\r\n"
    "3arXEAieUjKDvrJrrzbqFkNsaoEfQS1vfp+xjcut2I/qpcuSXW6rb4XlZotYgM5o\r\n"
    "q5So/tkkBrkr+G0VR1niD38hh9FQmO9ubvfTsDoT62cMruwhEAVQ7a7kUcrXdpqk\r\n"
    "Bu/fQQWvjaD7BIiPIbbUI2k5WCTyppYTXBWaoCH/5tEHXQZpJmL+Ccv/KNBlasF8\r\n"
    "nq1hAgMBAAGjUDBOMB0GA1UdDgQWBBRtJcMI1WZ7/c9AIkpjD1Ft8tKq5TAfBgNV\r\n"
    "HSMEGDAWgBSY7X3oJPBt3AIqxwKNo2aHXNJllDAMBgNVHRMEBTADAQH/MA0GCSqG\r\n"
    "SIb3DQEBCwUAA4IBAQARzarv5MZtQfy393K6H/ejwTpn8TYVq1MGLFQsksoZOh5z\r\n"
    "s4DKlJUwNhvNbksTe7WfdZJLhbTNvMQksAgMk+nlgT4vzs6aU0aJK73g+I5zVoFR\r\n"
    "51KXvZ0WxnPgqgNtdbo+t1wV/zEwJIDRo+2F81Jgd5xZt3hzV4sRY9Zyvl7GWg8s\r\n"
    "dlggFmc2dF7cBicQvWMynwyC0EcB52/bHi8VA4pzKykZtDSsy9waTxyVed/8p9gL\r\n"
    "wsZimijKZ7uLdbUBQCb2q5jGenHQFi4usiqn5WVHMYpbNlLrJkpHoHkEMyJIPYDH\r\n"
    "zcv2b5KXfhCOyQUtdv7fh1l5I4NFp+NLNcl1c+mJ\r\n"
    "-----END CERTIFICATE-----\r\n"
);

const ByteArray cert2Crl(
    "-----BEGIN X509 CRL-----\r\n"
    "MIIBZTBPAgEBMA0GCSqGSIb3DQEBCwUAMA0xCzAJBgNVBAMMAmNhFw0yNjAxMTEx\r\n"
    "MTMxNDlaFw0zNjAxMDkxMTMxNDlaoA4wDDAKBgNVHRQEAwIBATANBgkqhkiG9w0B\r\n"
    "AQsFAAOCAQEAA60pJibDv/pIw+dEG6XYMwmjjynduCjClZXmwxuAcFFO6kYiQNuz\r\n"
    "e76ipbBTkEB6DBel70SBcwDm6TeEqAfpM3kyV93a7t7xL+WXtTf7+Ru0daWxbuj6\r\n"
    "cMO/N/OD1KIDOvfHJjYS+q4HwJ7t5W1lUCCpKXVZEkCCYXF8KA0S4p0Wy6vTcaMF\r\n"
    "y42lRUeDoIyVVJ37nzYLihOBlxKL+Datn3JFaGHyibFSAI2EsoZwPUMOd7zXbjY6\r\n"
    "7z04ErYEmd8NAuGHdugfvouc/ybpodsZCyTSWtnufRwmsQYwefIRbJSu/AzOtUGR\r\n"
    "GZdjViZURAdw1+zgszO7Y6ZZrto+gGxy+g==\r\n"
    "-----END X509 CRL-----\r\n"
);

const ByteArray cert3(
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDADCCAeigAwIBAgIUR6hzmRwZpLodPwaeXfL1mBfhRBIwDQYJKoZIhvcNAQEL\r\n"
    "BQAwETEPMA0GA1UEAwwGcm9vdGNhMB4XDTI2MDExMTExMzE0OVoXDTM2MDEwOTEx\r\n"
    "MzE0OVowETEPMA0GA1UEAwwGcm9vdGNhMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A\r\n"
    "MIIBCgKCAQEAqpiIdyQrfN8ikaCIKO3X3v2kqvactX0QX9fL5lJ1RlA1VoCC1e4i\r\n"
    "6ZCFDrUNg6ZnNV5xwnGKRRq274+sIvZ6eu2TVOrakLYXjA8lgjE4uhun1IwS4QAx\r\n"
    "nuOlY48AJJiQ9kyP9SPARVI7YW7Chhe1qktKaP2Ego1AgSuOibOR7eCnpu1/IQUS\r\n"
    "OulujVSj1oWEHRq7gt+YadKY4mhEN6kI7msMXcr8Us4O3ylUMA663duSUrBYcL6c\r\n"
    "8cN5jcZBD8u2uWmjbmW/2ArGmv3xBQc2KowoCjvCGVc9eVrdyefpWSRXijAGG+7G\r\n"
    "pNhfMW/uAL/ggPcM+Chns/IMFGbHtvrYQwIDAQABo1AwTjAdBgNVHQ4EFgQUmO19\r\n"
    "6CTwbdwCKscCjaNmh1zSZZQwHwYDVR0jBBgwFoAUmO196CTwbdwCKscCjaNmh1zS\r\n"
    "ZZQwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEALdglHc63Cf84xbW9\r\n"
    "SPK+dDZ748OYgtrhjXTPtiwoZggTC5jaG/sVSJ5T+O/NHhpFpsDCQnZ8oYYucC7d\r\n"
    "BSeJh3BJB74JCE8jBmusJ02evmouxnzS7Ae2nafkRcc0yXcxLQZDwbXA9ZAZgrtQ\r\n"
    "Gos2BkWJMRdZJLtK256LbpCySOTbTFGTsHKtMDHEyX2ss13mFUHN1PBKQ6reLWl7\r\n"
    "u8sD0dQqVmOyS7lvXF2EkxC2aMsG/bCDTsXrASetPGdh85fPbUm13rKNA/Q6NxEt\r\n"
    "AV+ySb4KYvvwou3e3wGKjY+n36raSCMhOGv6sVcyCHFlWxyEj7OHVTnym4eRkH1H\r\n"
    "VTgnag==\r\n"
    "-----END CERTIFICATE-----\r\n"
);

inline ByteArray detach(const ByteArray & ba)
{
    ByteArray rv(ba);
    rv.detach();
    return rv;
}

}
