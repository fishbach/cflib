/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "config.h"

#include <cflib/db/dbconfig.h>

namespace cflib { namespace dao {

Config * Config::instance_ = 0;

Config::Config() :
    isProduction(false),
    emailsEnabled(false)
{
    if (instance_ == 0) instance_ = this;
}

void Config::loadFromDB()
{
    const QMap<QString, QString> vals = cflib::db::getConfig();

    isProduction  = vals["isProduction"] == "true";
    emailsEnabled = vals["emailsEnabled"] == "true";
    baseURL       = vals["baseURL"];

    init(vals);
}

}}
