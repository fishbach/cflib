/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::util {

class TimeShifter
{
public:
    static void setDateTime(const DateTime & newNow = DateTime());
    static DateTime currentDateTime();    // returns UTC

private:
    static cfint64 diff_;
};

} // namespace
