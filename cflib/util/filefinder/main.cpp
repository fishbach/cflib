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
	QTextStream(stdout) << "\"" << files.join("\" \"") << "\"" << endl;
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
