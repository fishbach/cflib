/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "test.h"

#include <cflib/util/log.h>

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
    std::sort(retval.begin(), retval.end(), classNameLessThan);
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
    std::sort(retval.begin(), retval.end());
    return retval;
}

}}    // namespace

using namespace cflib::util;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList args = a.arguments();
    QString runOnly;
    if (args.size() > 1 && !args[1].startsWith('-')) {
        runOnly = args[1].toLower();
        bool found = false;
        foreach (const QString & t, allTests()) if (t.toLower() == runOnly) { found = true; break; }
        if (!found) {
            QTextStream err(stderr);
            err << "Class \"" << args[1] << "\" not found!" << Qt::endl
                << "Existing classes:" << Qt::endl;
            foreach (const QString & t, allTests()) err << "  " << t << Qt::endl;
            return 1;
        }
        args.removeAt(1);
    }

    Log::start("test.log");

    QTextStream out(stdout);
    int retval = 0;
    bool first = true;
    foreach (QObject * obj, sortedTestObjects()) {
        if (!runOnly.isEmpty() && QString(obj->metaObject()->className()).toLower() != runOnly) continue;
        if (first) first = false;
        else       out << Qt::endl;
        retval += QTest::qExec(obj, args);
    }

    return retval;
}
