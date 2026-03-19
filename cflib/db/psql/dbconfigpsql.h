/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/mailer.h>

namespace cflib { namespace db {

CFMap<String, String> getConfigPSql();
cflib::util::Mail getMailTemplatePSql(const String & name, const String & lang);

}}    // namespace
