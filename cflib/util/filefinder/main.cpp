/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <QtCore>

int usage()
{
	QTextStream(stderr)
		<< "usage: " << QCoreApplication::applicationName() << " <path> [!] <regex>" << endl;

	return 1;
}

QStringList getFiles(const QDir & dir)
{
	QStringList retval;
	foreach (const QFileInfo & fi, dir.entryInfoList(QDir::Files | QDir::Readable)) {
		if (!fi.isSymLink()) retval << fi.filePath();
	}
	foreach (const QFileInfo & fi, dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable)) {
		if (!fi.isSymLink()) retval += getFiles(fi.filePath());
	}
	return retval;
}

int findFiles(const QString & path, bool inv, const QString & regex)
{
	const QRegExp re(regex);
	if (!re.isValid()) {
		QTextStream(stderr) << "invalid regex: " << regex << endl;
		return 2;
	}

	QStringList files = getFiles(path);
	QMutableStringListIterator it(files);
	while (it.hasNext()) if ((re.indexIn(it.next()) == -1) != inv) it.remove();
	files.sort();
	QTextStream out(stdout);
	foreach (const QString & file, files) {
		out << file << endl;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	QStringList args = app.arguments();
	args.removeFirst();

	if (args.size() == 2)                   return findFiles(args[0], false, args[1]);
	if (args.size() == 3 && args[1] == "!") return findFiles(args[0], true,  args[2]);

	return usage();
}
