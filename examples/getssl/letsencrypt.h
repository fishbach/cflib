/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <QtCore>

class LetsEncrypt
{
	Q_DISABLE_COPY(LetsEncrypt)
public:
	LetsEncrypt(const QList<QByteArray> & domains, const QByteArray & email,
		const QByteArray & privateKeyFile, const QByteArray & destDir,
		bool test);
	~LetsEncrypt();

	void start();

private:
	class Impl;
	Impl * impl_;
};
