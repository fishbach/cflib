/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#pragma once

#include <QtTest/QtTest>

#define ADD_TEST(Class) \
	namespace { \
		int cflib_util_test_add_##Class() { cflib::util::addTest(new Class); return 0; } \
		Q_CONSTRUCTOR_FUNCTION(cflib_util_test_add_##Class) \
	}

namespace cflib { namespace util {

void addTest(QObject * test);
QStringList allTests();

}}	// namespace
