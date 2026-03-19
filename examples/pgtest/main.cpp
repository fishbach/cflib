/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/db/psql/psql.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>

#include <format>
#include <iostream>

using namespace cflib::db;
using namespace cflib::util;

USE_LOG(LogCat::Db)

namespace {

cfuint32 random(cfuint32 min, cfuint32 max)
{
    const cfuint32 count = max - min + 1;
    const cfuint32 div   = RAND_MAX / count;
    const cfuint32 rest  = div * count;

    cfuint32 r;
    while ((r = (cfuint32)rand()) >= rest);

    return r / div + min;
}

void insert(int start, int end)
{
    PSqlConn;
    sql.prepare("INSERT INTO test VALUES ($14, $15, $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)");
    sql << 2.0 << 3.0 << 4.0 << 5.0 << 6.0 << 7.0 << 8.0 << 9.0 << 10.0 << 11.0 << 12.0 << 13.0 << 14.0;
    for (int i = start ; i <= end ; ++i) {
        sql << (cfuint32)i << CFDateTime::currentDateTimeUtc();
        sql.exec(13);
    }
}

void select()
{
    PSqlConn;
    sql.prepare("SELECT * FROM test");
    sql.exec();

    uint count = 0;
    cfuint32 i;
    CFDateTime t;
    double d;
    while (sql.next()) {
        sql >> i >> t >> d >> d >> d >> d >> d >> d >> d >> d >> d >> d >> d >> d >> d;
        ++count;
    }
    std::cout << std::format("count: {}\n", count);
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

    static cfint64 last      = CFDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    static cfint64 lastMicro = 0;
    static cfint64 count     = 0;
    static cfint64 minLat    = 0x7fffffff;
    static cfint64 maxLat    = 0;
    static cfint64 sum       = 0;

    while (true) {
        sql << random(start, end) << CFDateTime::currentDateTimeUtc();
        sql.exec(13);

        cfint64 nowMicro = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        cfint64 latency = nowMicro - lastMicro;
        lastMicro = nowMicro;

        if (latency == nowMicro) continue;

        if (minLat > latency) minLat = latency;
        if (maxLat < latency) maxLat = latency;
        sum += latency;
        ++count;

        cfint64 now = CFDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        if (now - last > 1000) {
            std::cout << std::format("{} msg/s - latency: {} / {} / {} microsec\n",
                (long long)(count * 1000 / (now - last)),
                (long long)minLat, (long long)(sum / count), (long long)maxLat);
            last  = now;
            count = 0;
            minLat = 0x7fffffff;
            maxLat = 0;
            sum   = 0;
        }
    }
}

int showUsage(const CFByteArray & executable)
{
    std::cerr << std::format(
        "Usage: {} [options] 'host=... port=...'\n"
        "Options:\n"
        "  -h, --help       => this help\n"
        "  -i <start>-<end> => insert elements with ids >= start and <= end\n"
        "  -s               => select\n"
        "  -u <start>-<end> => update\n"
        "\n"
        "CREATE TABLE test (\n"
        "  id        integer NOT NULL,\n"
        "  timestamp timestamp with time zone,\n"
        "  x         double precision NOT NULL,\n"
        "  y         double precision NOT NULL,\n"
        "  z         double precision NOT NULL,\n"
        "  qw        double precision NOT NULL,\n"
        "  qx        double precision NOT NULL,\n"
        "  qy        double precision NOT NULL,\n"
        "  qz        double precision NOT NULL,\n"
        "  latitude  double precision NOT NULL,\n"
        "  longitude double precision NOT NULL,\n"
        "  globalQw  double precision NOT NULL,\n"
        "  globalQx  double precision NOT NULL,\n"
        "  globalQy  double precision NOT NULL,\n"
        "  globalQz  double precision NOT NULL,\n"
        "  PRIMARY KEY (id)\n"
        ");\n",
        executable.constData());
    return 1;
}

}

int main(int argc, char *argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help     ('h', "help"                      ); cmdLine << help;
    Option insertOpt('i', CFByteArray(), true          ); cmdLine << insertOpt;
    Option selectOpt('s'                              ); cmdLine << selectOpt;
    Option updateOpt('u', CFByteArray(), true          ); cmdLine << updateOpt;
    Arg    sqlParam (false);                             cmdLine << sqlParam;
    if (!cmdLine.parse() || help.isSet() ||
        (insertOpt.isSet() ? 1 : 0) +
        (selectOpt.isSet() ? 1 : 0) +
        (updateOpt.isSet() ? 1 : 0) != 1) return showUsage(cmdLine.executable());

    Log::start("pgtest.log");
    Log::setLogLevel(LogCat::Info);

    PSql::setParameter(String(sqlParam.value()));

    CFElapsedTimer timer;

    if (insertOpt.isSet()) {
        CFByteArray val = insertOpt.value();
        cfsize_t sep = val.indexOf('-');
        insert(val.mid(0, sep).toInt(), val.mid(sep + 1).toInt());
    }
    if (selectOpt.isSet()) select();
    if (updateOpt.isSet()) {
        CFByteArray val = updateOpt.value();
        cfsize_t sep = val.indexOf('-');
        update(val.mid(0, sep).toInt(), val.mid(sep + 1).toInt());
    }

    std::cout << std::format("elapsed: {}\n", (long long)timer.elapsed());

    return 0;
}
