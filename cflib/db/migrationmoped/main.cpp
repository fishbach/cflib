/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/db/psql.h>
#include <cflib/db/schema.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>

using namespace cflib::db;
using namespace cflib::util;

USE_LOG(LogCat::Db)

namespace {

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Usage: " << executable << " [options] <db schema file>"       << Qt::endl
		<< "Options:"                                                     << Qt::endl
		<< "  -h, --help             => this help"                        << Qt::endl
		<< "  -d, --db <param>       => set DB parameters"                << Qt::endl
		<< "  -m, --migrator <param> => set migrator executable"          << Qt::endl
		<< "  -l, --log <level>      => set log level 0 -> all, 7 -> off" << Qt::endl;
	return 1;
}

}

int main(int argc, char *argv[])
{
	CmdLine cmdLine(argc, argv);
	Option help       ('h', "help"          ); cmdLine << help;
	Option dbOpt      ('d', "db",       true); cmdLine << dbOpt;
	Option migratorOpt('m', "migrator", true); cmdLine << migratorOpt;
	Option logOpt     ('l', "log",      true); cmdLine << logOpt;
	Arg    schemaArg  (false                ); cmdLine << schemaArg;
	if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

	QCoreApplication a(argc, argv);

	// start logging
	if (logOpt.isSet()) {
		Log::start("migrationmoped.log");
		logInfo("migrationmoped started");
		Log::setLogLevel(logOpt.value().toUInt());
	}

	PSql::setParameter(dbOpt.isSet() ? dbOpt.value() : "");
	if (!schema::update(!migratorOpt.isSet() ? schema::Migrator() :
		[&](const QByteArray & name) {
			return QProcess::execute(QString::fromUtf8(migratorOpt.value()), QStringList() << QString::fromUtf8(name)) == 0;
		}, QString::fromUtf8(schemaArg.value())))
	{
		logCritical("could not update db schema");
		QTextStream(stderr) << "could not update db schema" << Qt::endl;
		return 1;
	}

	return 0;
}
