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

#include <cflib/crypt/tlscredentials.h>
#include <cflib/crypt/util.h>
#include <cflib/util/test.h>

#include "certs.h"

using namespace cflib::crypt;

class TLSCredentials_test : public QObject
{
	Q_OBJECT
private slots:

	void test_addCerts()
	{
		TLSCredentials creds;

		QCOMPARE((int)creds.addCerts(QByteArray()), 0);
		QCOMPARE((int)creds.addCerts(cert3 + cert1), 2);
		QCOMPARE((int)creds.addCerts(cert1), 0);
		QCOMPARE((int)creds.addCerts(cert2), 1);
		QVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));
		QVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey), "wrong"));
		QVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

		const QList<TLSCertInfo> infos = creds.getCertInfos();
		QCOMPARE(infos.size(), 3);
		QCOMPARE(infos[0].subjectName.constData(), "127.0.0.1");
		QCOMPARE(infos[0].issuerName .constData(), "ca");
		QVERIFY(!infos[0].isCA);
		QVERIFY(!infos[0].isTrusted);
		QCOMPARE(infos[1].subjectName.constData(), "ca");
		QCOMPARE(infos[1].issuerName .constData(), "rootca");
		QVERIFY( infos[1].isCA);
		QVERIFY(!infos[1].isTrusted);
		QCOMPARE(infos[2].subjectName.constData(), "rootca");
		QCOMPARE(infos[2].issuerName .constData(), "rootca");
		QVERIFY( infos[2].isCA);
		QVERIFY(!infos[2].isTrusted);
	}

	void test_addCerts_trusted()
	{
		TLSCredentials creds;

		QCOMPARE((int)creds.addCerts(cert2 + cert1), 2);
		QCOMPARE((int)creds.addCerts(cert3, true), 1);
		QVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));

		const QList<TLSCertInfo> infos = creds.getCertInfos();
		QCOMPARE(infos.size(), 3);
		QCOMPARE(infos[0].subjectName.constData(), "127.0.0.1");
		QVERIFY(!infos[0].isTrusted);
		QCOMPARE(infos[1].subjectName.constData(), "ca");
		QVERIFY(!infos[1].isTrusted);
		QCOMPARE(infos[2].subjectName.constData(), "rootca");
		QVERIFY( infos[2].isTrusted);
	}

	void test_setPrivateKey()
	{
		TLSCredentials creds;

		QVERIFY(!creds.addPrivateKey(QByteArray()));
		QVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));

		QCOMPARE((int)creds.addCerts(cert2), 1);
		QVERIFY(!creds.addPrivateKey(detach(cert1PrivateKey)));

		QCOMPARE((int)creds.addCerts(cert1), 1);
		QVERIFY(creds.addPrivateKey(detach(cert1PrivateKey), "SuperSecure123"));
	}

};
#include "tlscredentials_test.moc"
ADD_TEST(TLSCredentials_test)
