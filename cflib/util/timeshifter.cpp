/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "timeshifter.h"

namespace cflib { namespace util {

cfint64 TimeShifter::diff_ = 0;

void TimeShifter::setDateTime(const CFDateTime & newNow)
{
    if (newNow.isNull()) diff_ = 0;
    else diff_ = CFDateTime::nowUTC().msecsTo(newNow);
}

CFDateTime TimeShifter::currentDateTime()
{
    return CFDateTime::nowUTC().addMSecs(diff_);
}

}}    // namespace
