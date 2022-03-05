/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
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

}}	// namespace
