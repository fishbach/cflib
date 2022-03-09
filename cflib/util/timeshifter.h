/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace util {

class TimeShifter
{
public:
	static void setDateTime(const QDateTime & newNow = QDateTime());
	static QDateTime currentDateTime();	// returns UTC

private:
	static qint64 diff_;
};

}}	// namespace
