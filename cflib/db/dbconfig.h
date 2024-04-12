/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/mailer.h>

namespace cflib { namespace db {

QMap<QString, QString> getConfig();
cflib::util::Mail getMailTemplate(const QString & name, const QString & lang);

}}    // namespace
