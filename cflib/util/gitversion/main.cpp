/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <QtCore>

int usage()
{
    QTextStream(stderr) << "usage: " << QCoreApplication::applicationName()
                        << " <create> <git repo path> <path to gitversion.h>" << Qt::endl;

    return 1;
}

int createHeader(const QString & searchPath, const QString & filename)
{
    QProcess git;

    git.setWorkingDirectory(searchPath);
    git.start("git", {"rev-parse", "HEAD"});
    if (!git.waitForStarted()) {
        QTextStream(stderr) << "cannot start git rev-parse HEAD" << Qt::endl;
        return 3;
    }
    if (!git.waitForFinished()) {
        QTextStream(stderr) << "git rev-parse HEAD failed" << Qt::endl;
        return 4;
    }
    QByteArray hash = git.readAll();
    if (hash.isEmpty()) {
        QTextStream(stderr) << "cannot determine git hash" << Qt::endl;
        return 5;
    }
    hash = hash.trimmed();

    QFile in(filename);
    if (in.open(QFile::ReadOnly)) {
        QString tail = QString::fromLatin1(in.readAll());
        in.close();

        // Extract existing hash
        QString existing_hash;
        QRegExp re("GIT_VERSION \"([0-9a-f]{40})\"");
        int pos = re.indexIn(tail);
        while (pos != -1) {
            existing_hash = re.cap(1);
            pos = re.indexIn(tail, pos + re.matchedLength());
        }

        // Nothing to change
        if (hash == existing_hash) {
            return 0;
        }
    }
    QFile out(filename);
    if (!out.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream(stderr) << "cannot write: " << out.fileName() << Qt::endl;
        return 4;
    }
    QTextStream(&out)
        << "#pragma once" << Qt::endl
        << Qt::endl
        << "#define GIT_VERSION \"" << hash << "\"" << Qt::endl;
    out.close();
    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    args.removeFirst();

    if (args.size() == 3 && args[0] == "create") return createHeader(args[1], args[2]);

    return usage();
}
