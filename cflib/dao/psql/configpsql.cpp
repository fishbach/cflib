/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "configpsql.h"

#include <cflib/db/psql/dbconfigpsql.h>

namespace cflib::dao {

ConfigPSql * ConfigPSql::instance_ = 0;

ConfigPSql::ConfigPSql() :
    isProduction(false),
    emailsEnabled(false)
{
    if (instance_ == 0) instance_ = this;
}

void ConfigPSql::loadFromDB()
{
    const Map<String, String> vals = cflib::db::getConfigPSql();

    auto it = vals.find(String("isProduction"));
    isProduction = (it != vals.end() && it->second == "true");

    it = vals.find(String("emailsEnabled"));
    emailsEnabled = (it != vals.end() && it->second == "true");

    it = vals.find(String("baseURL"));
    baseURL = (it != vals.end()) ? it->second : String();

    init(vals);
}

}