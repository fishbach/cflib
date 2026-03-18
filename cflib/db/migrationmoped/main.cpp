/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/db/psql/psql.h>
#include <cflib/db/psql/schema.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

#include <cstdlib>

using namespace cflib::db;
using namespace cflib::util;

USE_LOG(LogCat::Db)

namespace {

int showUsage(const CFByteArray & executable)
{
    fprintf(stderr,
        "Usage: %s [options] <db schema file>\n"
        "Options:\n"
        "  -h, --help             => this help\n"
        "  -d, --db <param>       => set DB parameters\n"
        "  -m, --migrator <param> => set migrator executable\n"
        "  -l, --log <level>      => set log level 0 -> all, 7 -> off\n",
        executable.constData());
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

    // start logging
    if (logOpt.isSet()) {
        Log::start("migrationmoped.log");
        logInfo("migrationmoped started");
        Log::setLogLevel(logOpt.value().toUInt());
    }

    CFString dbParam = dbOpt.isSet() ? CFString::fromUtf8(dbOpt.value()) : CFString();
    PSql::setParameter(dbParam);

    schema::Migrator migrator;
    if (migratorOpt.isSet()) {
        CFString migratorExe = CFString::fromUtf8(migratorOpt.value());
        migrator = [&migratorExe](const CFByteArray & name) {
            CFString cmd = migratorExe + " " + CFString::fromUtf8(name);
            return system(cmd.c_str()) == 0;
        };
    }

    CFString schemaFile = CFString::fromUtf8(schemaArg.value());
    if (!schema::update(migrator, schemaFile))
    {
        logCritical("could not update db schema");
        fprintf(stderr, "could not update db schema\n");
        return 1;
    }

    return 0;
}
