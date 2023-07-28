/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
