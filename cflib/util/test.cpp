/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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

#include "test.h"

namespace cflib { namespace util {

namespace {

Q_GLOBAL_STATIC(QList<QObject *>, testObjectsQGS)

bool classNameLessThan(const QObject * a, const QObject * b)
{
	if (a->metaObject()->className() == b->metaObject()->className()) return a < b;
	return a->metaObject()->className() < b->metaObject()->className();
}

QList<QObject *> & sortedTestObjects()
{
	QList<QObject *> & retval = *testObjectsQGS();
	qSort(retval.begin(), retval.end(), classNameLessThan);
	return retval;
}

}

void addTest(QObject * test)
{
	testObjectsQGS()->append(test);
}

QStringList allTests()
{
	QStringList retval;
	foreach (QObject * obj, sortedTestObjects()) {
		retval << obj->metaObject()->className();
	}
	return retval;
}

}}	// namespace

using namespace cflib::util;

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QTextStream out(stdout);
	int retval = 0;
	bool first = true;
	foreach (QObject * obj, sortedTestObjects()) {
		if (first) first = false;
		else       out << endl;
		retval += QTest::qExec(obj, argc, argv);
	}

	return retval;
}
