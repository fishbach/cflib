/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "configpsql.h"

#include <cflib/db/psql/dbconfigpsql.h>

namespace cflib { namespace dao {

ConfigPSql * ConfigPSql::instance_ = 0;

ConfigPSql::ConfigPSql() :
    isProduction(false),
    emailsEnabled(false)
{
    if (instance_ == 0) instance_ = this;
}

void ConfigPSql::loadFromDB()
{
    const CFMap<CFString, CFString> vals = cflib::db::getConfigPSql();

    auto it = vals.find(CFString("isProduction"));
    isProduction = (it != vals.end() && it->second == "true");

    it = vals.find(CFString("emailsEnabled"));
    emailsEnabled = (it != vals.end() && it->second == "true");

    it = vals.find(CFString("baseURL"));
    baseURL = (it != vals.end()) ? it->second : CFString();

    init(vals);
}

}}
