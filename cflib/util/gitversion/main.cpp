/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <QtCore>

int usage()
{
	QTextStream(stderr)
		<< "usage: " << QCoreApplication::applicationName() << " <depend|create> <.git search path> <gitversion.h>" << Qt::endl;

	return 1;
}

QString findGitDir(const QString & searchPath, int & retval)
{
	QDir dir(searchPath);
	QFileInfo fi = dir.absoluteFilePath(".git");
	while (!fi.isDir()) {
		if (!dir.cdUp()) {
			QTextStream(stderr) << "cannot find .git directory" << Qt::endl;
			retval = 1;
			return QString();
		}
		fi = dir.absoluteFilePath(".git");
	}
	if (!fi.isReadable()) {
		QTextStream(stderr) << "cannot read .git directory: " << fi.canonicalFilePath() << Qt::endl;
		retval = 2;
		return QString();
	}
	return fi.canonicalFilePath();
}

int createHeader(const QString & searchPath, const QString & filename)
{
	int retval;
	QString gitDir = findGitDir(searchPath, retval);
	if (gitDir.isNull()) return retval;

	// fetch tail
	QFile file(gitDir + "/logs/HEAD");
	if (!file.open(QFile::ReadOnly)) {
		QTextStream(stderr) << "cannot read: " << file.fileName() << Qt::endl;
		return 3;
	}
	if (file.size() > 1024) file.seek(file.size() - 1024);
	QString tail = QString::fromLatin1(file.readAll());
	file.close();

	// extract hash
	QString hash;
	QRegExp re("(?:^|\n)\\s*[0-9a-f]{40} ([0-9a-f]{40}) ");
	int pos = re.indexIn(tail);
	while (pos != -1) {
		hash = re.cap(1);
		pos = re.indexIn(tail, pos + re.matchedLength());
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

int depend(const QString & searchPath)
{
	int retval;
	QString gitDir = findGitDir(searchPath, retval);
	if (gitDir.isNull()) return retval;

	QTextStream(stdout) << gitDir << "/logs/HEAD" << Qt::endl;
	return 0;
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	QStringList args = app.arguments();
	args.removeFirst();

	if (args.size() == 2 && args[0] == "depend") return depend(args[1]);
	if (args.size() == 3 && args[0] == "create") return createHeader(args[1], args[2]);

	return usage();
}
