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

inline QString & operator<<(QString & lhs, const QString & rhs) { return lhs += rhs; }
inline QString & operator<<(QString & lhs, const char * rhs) { return lhs += rhs; }

const QRegularExpression callRE(R"((?:^|\W)(require|define)\s*\((\s*|\s*\[.*?\]\s*,\s*)function\s*\()",
	QRegularExpression::DotMatchesEverythingOption);
const QRegularExpression closingRE(R"(\(|\)|"|'|//|/\*)",
	QRegularExpression::DotMatchesEverythingOption);
const QRegularExpression eolRE(R"(\r|\n)",
	QRegularExpression::DotMatchesEverythingOption);

QString output;

QString basePath = "";

struct Space {
	QMap<QString, Space> sub;
};
Space spaces;

int usage()
{
	QTextStream(stderr) << "usage: " << QCoreApplication::applicationName() << " <main.js>" << endl;
	return 1;
}

int findClosing(const QString & src, int start)
{
	const int len = src.length();
	int openCount = 0;
	forever {
		QRegularExpressionMatch m = closingRE.match(src, start);
		if (!m.hasMatch()) break;
		start = m.capturedEnd();

		QString found = m.captured();
		if (found == "(") {
			++openCount;
		} else if (found == ")") {
			if (openCount == 0) return start;
			--openCount;
		} else if (found == "'") {
			int pos = start;
			while (pos < len) {
				QChar c = src[pos++];
				if (c == '\\') ++pos;
				else if (c == '\'') {
					start = pos;
					break;
				}
			}
		} else if (found == "\"") {
			int pos = start;
			while (pos < len) {
				QChar c = src[pos++];
				if (c == '\\') ++pos;
				else if (c == '"') {
					start = pos;
					break;
				}
			}
		} else if (found == "//") {
			int pos = src.indexOf(eolRE, start);
			if (pos == -1) break;
			start = pos + 1;
		} else if (found == "/*") {
			int pos = src.indexOf("*/", start);
			if (pos == -1) break;
			start = pos + 1;
		}
	}

	QTextStream(stderr) << "cannot find closing bracket" << endl;
	return len - 1;
}

QStringList parseDepends(const QString & depends)
{
	QStringList rv;
	int pos = 0;
	int start = -1;
	while (pos < depends.length()) {
		QChar c = depends[pos++];
		if (c == '\'' || c == '"') {
			if (start == -1) start = pos;
			else {
				rv << depends.mid(start, pos - start - 1);
				start = -1;
			}
		}
	}
	return rv;
}

void getDependencies(const QString & name)
{
	QString file;
	{
		QFile f(name);
		f.open(QFile::ReadOnly);
		file = QString::fromUtf8(f.readAll());
	}
	int pos = 0;
	forever {
		QRegularExpressionMatch m = callRE.match(file, pos);
		if (!m.hasMatch()) break;
		pos = m.capturedEnd();

		if (m.captured(1) == "define") {
			QStringList parts = name.mid(basePath.length()).split('/');
			parts.removeLast();
			Space * sp = &spaces;
			foreach (const QString & p, parts) sp = &(sp->sub[p]);
		}

		foreach (const QString & dep, parseDepends(m.captured(2))) getDependencies(basePath + dep + ".js");
	}
}

void convertFile(const QString & name)
{
	QString file;
	{
		QFile f(name);
		f.open(QFile::ReadOnly);
		file = QString::fromUtf8(f.readAll());
	}
	int pos = 0;
	forever {
		QRegularExpressionMatch m = callRE.match(file, pos);
		if (!m.hasMatch()) break;
		pos = m.capturedEnd();

//		QTextStream(stdout) << m.captured(1) << " | " << m.captured(2) << " | " << m.captured(3) << "\n";
		int start = m.capturedStart(1);
		int end = m.capturedEnd(2);
		int closing = findClosing(file, end);

		file.insert(closing, "C");
		file.insert(end, "Y");
		file.insert(start, "X");

	}
//	output += file;
}

void printSpaces(const Space & space)
{
	if (space.sub.isEmpty()) {
		output += "{}";
		return;
	}
	output += "{ ";
	bool first = true;
	QMapIterator<QString, Space> it(space.sub);
	while (it.hasNext()) {
		it.next();
		if (first) first = false;
		else       output += ", ";
		output << it.key() << ": ";
		printSpaces(it.value());
	}
	output += " }";
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	QStringList args = app.arguments();
	args.removeFirst();

	if (args.size() != 1) return usage();
	QString main = args.first();
	if (!QFileInfo(main).isReadable()) return usage();

	int pos = main.lastIndexOf('/');
	if (pos != -1) basePath = main.left(pos + 1);

	getDependencies(main);
	output += "var mod = ";
	printSpaces(spaces);
	output += ";\n";
//	convertFile(main);

	QTextStream(stdout) << output.toUtf8();
	return 0;
}
