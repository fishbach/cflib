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

#include <cflib/util/cmdline.h>
#include <cflib/util/util.h>

using namespace cflib::util;

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

QSet<QString> available;
QStringList order;
QSet<QString> defined;
QSet<QString> excludes;

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
	QString file = readTextfile(name);

	int pos = 0;
	forever {
		QRegularExpressionMatch m = callRE.match(file, pos);
		if (!m.hasMatch()) break;
		pos = m.capturedEnd();

		// update spaces
		if (m.captured(1) == "define") {
			const QString mod = name.mid(basePath.length(), name.length() - basePath.length() - 3);
			defined << mod;
			QStringList parts = mod.split('/');
			parts.removeLast();
			Space * sp = &spaces;
			foreach (const QString & p, parts) sp = &(sp->sub[p]);
		}

		foreach (const QString & dep, parseDepends(m.captured(2))) {
			if (excludes.contains(dep)) continue;
			const QString depFile = basePath + dep + ".js";
			if (!available.contains(depFile)) getDependencies(depFile);
		}
	}

	available << name;
	order << name;
}

void convertFile(const QString & name)
{
	QString file = readTextfile(name);

	int pos = 0;
	forever {
		QRegularExpressionMatch m = callRE.match(file, pos);
		if (!m.hasMatch()) break;
		pos = m.capturedEnd();

		int start   = m.capturedStart(1);
		int end     = m.capturedEnd(2);
		int closing = findClosing(file, end);

		QString params = "(";
		bool first = true;
		foreach (QString dep, parseDepends(m.captured(2))) {
			if (first) first = false;
			else params += ", ";
			if (defined.contains(dep)) params << "mod." << dep.replace('/', '.');
			else                       params << "null";
		}
		params += ")";
		file.insert(closing, params);

		QString head;
		if (m.captured(1) == "define") {
			QString mod = name.mid(basePath.length(), name.length() - basePath.length() - 3);
			head << "mod." << mod.replace('/', '.') << " = ";
		}
		head << "(";
		file.replace(start, end - start, head);
		pos += head.length() - end + start;
	}
	output += file;
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

	// parse cmd line
	CmdLine cmdLine(argc, argv);
	Option help    ('h', "help"                     ); cmdLine << help;
	Option exclude ('e', "exclude", true, true, true); cmdLine << exclude;
	Arg    fileName(false                           ); cmdLine << fileName;
	if (!cmdLine.parse() || help.isSet()) return usage();

	QString main = fileName.value();
	if (!QFileInfo(main).isReadable()) return usage();

	foreach (const QByteArray & a, exclude.values()) excludes << QString::fromUtf8(a);

	int pos = main.lastIndexOf('/');
	if (pos != -1) basePath = main.left(pos + 1);

	getDependencies(main);

	output += "var mod = ";
	printSpaces(spaces);
	output += ";\n";

	foreach (const QString & name, order) {
		output
			<< "\n// ============================================================================n"
			<< "// " << name << "\n"
			<< "// ============================================================================n\n";
		convertFile(name);
		output += "\n";
	}

	QTextStream(stdout) << output.toUtf8();
	return 0;
}
