/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "timeshifter.h"

namespace cflib { namespace util {

qint64 TimeShifter::diff_ = 0;

void TimeShifter::setDateTime(const QDateTime & newNow)
{
    if (newNow.isNull()) diff_ = 0;
    else diff_ = QDateTime::currentDateTimeUtc().msecsTo(newNow);
}

QDateTime TimeShifter::currentDateTime()
{
    return QDateTime::currentDateTimeUtc().addMSecs(diff_);
}

}}    // namespace
