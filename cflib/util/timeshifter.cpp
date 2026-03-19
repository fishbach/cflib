/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "timeshifter.h"

namespace cflib::util {

int64 TimeShifter::diff_ = 0;

void TimeShifter::setDateTime(const DateTime & newNow)
{
    if (newNow.isNull()) diff_ = 0;
    else diff_ = DateTime::nowUTC().msecsTo(newNow);
}

DateTime TimeShifter::currentDateTime()
{
    return DateTime::nowUTC().addMSecs(diff_);
}

} // namespace
