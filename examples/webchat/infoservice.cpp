/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "infoservice.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Compute)

InfoService::InfoService() :
	RMIService(serializeTypeInfo().typeName)
{
}

InfoService::~InfoService()
{
	stopVerifyThread();
}

QString InfoService::test()
{
	return QString::fromUtf8("hello w\xc3\xb6rld");
}

QString InfoService::test(const QString & msg)
{
	logInfo("msg %1: %2", msg.length(), msg);
	QTextStream(stdout) << msg << endl;
	return msg;
}

void InfoService::async(qint64 i)
{
	logInfo("async: %1", i);
}

Dao InfoService::update(const Dao & dao)
{
	Dao rv;
	rv.name = dao.name + "XX";
	rv.number = dao.number + 13;
	return rv;
}

void InfoService::update(Dao2 & dao)
{
	dao.numbers << 3 << 4 << 5;
}

void InfoService::update(Dao3 & dao)
{
	dao.timestamp = QDateTime::currentDateTimeUtc();
}

void InfoService::talk(const QString & msg)
{
	QTextStream(stdout) << QString("connId:%1\twrote: %2").arg(connId()).arg(msg) << endl;
	newMessage(connId(), msg);
}
