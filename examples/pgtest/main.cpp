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

#include <cflib/db/psql.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>

#include <chrono>

using namespace cflib::db;
using namespace cflib::util;

USE_LOG(LogCat::Db)

namespace {

QTextStream out(stdout);
QTextStream err(stderr);

quint32 random(quint32 min, quint32 max)
{
	const quint32 count = max - min + 1;
	const quint32 div = RAND_MAX / count;
	const quint32 rest = div * count;

	quint32 r;
	while ((r = (quint32)rand()) >= rest);

	return r / div + min;
}

void insert(int start, int end)
{
	PSqlConn;
	sql.prepare("INSERT INTO test VALUES ($14, $15, $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)");
	sql << 2.0 << 3.0 << 4.0 << 5.0 << 6.0 << 7.0 << 8.0 << 9.0 << 10.0 << 11.0 << 12.0 << 13.0 << 14.0;
	for (int i = start ; i <= end ; ++i) {
		sql << (quint32)i << QDateTime::currentDateTimeUtc();
		sql.exec(13);
	}
}

void select()
{
	PSqlConn;
	sql.prepare("SELECT * FROM test");
	sql.exec();

	uint count = 0;
	quint32 i;
	QDateTime t;
	double d;
	while (sql.next()) {
		sql >> i >> t >> d >> d >> d >> d >> d >> d >> d >> d >> d >> d >> d >> d >> d;
		++count;
	}
	out << "count: " << count << endl;
}

void update(int start, int end)
{
	PSqlConn;
	sql.prepare(
		"UPDATE test SET "
			"timestamp = $15, "
			"x = $1, y = $2, z = $3, "
			"qw = $4, qx = $5, qy = $6, qz = $7, "
			"latitude = $8, longitude = $9, "
			"globalQw = $10, globalQx = $11, globalQy = $12, globalQz = $13 "
		"WHERE "
			"id = $14"
	);
	sql << 2.0 << 3.0 << 4.0 << 5.0 << 6.0 << 7.0 << 8.0 << 9.0 << 10.0 << 11.0 << 12.0 << 13.0 << 14.0;

	forever {
		sql << random(start, end) << QDateTime::currentDateTimeUtc();
		sql.exec(13);

		static qint64 last = QDateTime::currentMSecsSinceEpoch();
		static qint64 lastMicro = 0;
		static qint64 count = 0;
		static qint64 min = 0x7fffffff;
		static qint64 max = 0;
		static qint64 sum = 0;
		qint64 nowMicro = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		qint64 latency = nowMicro - lastMicro;
		lastMicro = nowMicro;

		if (latency == nowMicro) continue;

		if (min > latency) min = latency;
		if (max < latency) max = latency;
		sum += latency;
		++count;

		qint64 now = QDateTime::currentMSecsSinceEpoch();
		if (now - last > 1000) {
			out
				<< (count * 1000 / (now - last)) << " msg/s - latency: "
				<< min << " / " << (sum / count) << " / " << max << " microsec" << endl;
			last = now;
			count = 0;
			min = 0x7fffffff;
			max = 0;
			sum = 0;
		}
	}
}

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Usage: " << executable << " [options] 'host=... port=...'"          << endl
		<< "Options:"                                                           << endl
		<< "  -h, --help       => this help"                                    << endl
		<< "  -i <start>-<end> => insert elements with ids >= start and <= end" << endl
		<< "  -s               => select"                                       << endl
		<< "  -u <start>-<end> => update"                                       << endl
		<< ""                                                                   << endl
		<< "CREATE TABLE test ("                                                << endl
		<< "  id        integer NOT NULL,"                                      << endl
		<< "  timestamp timestamp with time zone,"                              << endl
		<< "  x         double precision NOT NULL,"                             << endl
		<< "  y         double precision NOT NULL,"                             << endl
		<< "  z         double precision NOT NULL,"                             << endl
		<< "  qw        double precision NOT NULL,"                             << endl
		<< "  qx        double precision NOT NULL,"                             << endl
		<< "  qy        double precision NOT NULL,"                             << endl
		<< "  qz        double precision NOT NULL,"                             << endl
		<< "  latitude  double precision NOT NULL,"                             << endl
		<< "  longitude double precision NOT NULL,"                             << endl
		<< "  globalQw  double precision NOT NULL,"                             << endl
		<< "  globalQx  double precision NOT NULL,"                             << endl
		<< "  globalQy  double precision NOT NULL,"                             << endl
		<< "  globalQz  double precision NOT NULL,"                             << endl
		<< "  PRIMARY KEY (id)"                                                 << endl
		<< ");"                                                                 << endl;
	return 1;
}

}

int main(int argc, char *argv[])
{
	CmdLine cmdLine(argc, argv);
	Option help     ('h', "help"            ); cmdLine << help;
	Option insertOpt('i', QByteArray(), true); cmdLine << insertOpt;
	Option selectOpt('s'                    ); cmdLine << selectOpt;
	Option updateOpt('u', QByteArray(), true); cmdLine << updateOpt;
	Arg    sqlParam (false);                   cmdLine << sqlParam;
	if (!cmdLine.parse() || help.isSet() ||
		(insertOpt.isSet() ? 1 : 0) +
		(selectOpt.isSet() ? 1 : 0) +
		(updateOpt.isSet() ? 1 : 0) != 1) return showUsage(cmdLine.executable());

	QCoreApplication a(argc, argv);

	Log::start("pgtest.log");
	Log::setLogLevel(LogCat::Info);

	PSql::setParameter(QString::fromUtf8(sqlParam.value()));

	QElapsedTimer timer;
	timer.start();

	if (insertOpt.isSet()) insert(insertOpt.value().split('-').first().toInt(), insertOpt.value().split('-').last().toInt());
	if (selectOpt.isSet()) select();
	if (updateOpt.isSet()) update(updateOpt.value().split('-').first().toInt(), updateOpt.value().split('-').last().toInt());

	out << "elapsed: " << timer.elapsed() << endl;

	return 0;
}
