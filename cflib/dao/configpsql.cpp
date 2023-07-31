/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "configpsql.h"

#include <cflib/db/dbconfigpsql.h>

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
	const QMap<QString, QString> vals = cflib::db::getConfigPSql();

	isProduction  = vals["isProduction"] == "true";
	emailsEnabled = vals["emailsEnabled"] == "true";
	baseURL       = vals["baseURL"];

	init(vals);
}

}}
