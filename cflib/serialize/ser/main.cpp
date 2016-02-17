/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

#include <cflib/serialize/ser/codegen.h>
#include <cflib/serialize/ser/headerparser.h>

int usage()
{
	QTextStream(stderr)
		<< "usage: " << QCoreApplication::applicationName() << " serialize <header.h> <source_ser.cpp>" << endl;

	return 1;
}

int serialize(const QString & header, const QString & dest)
{
	QTextStream err(stderr);

	QFile in(header);
	if (!in.open(QIODevice::ReadOnly)) {
		err << "cannot read: " << header << endl;
		return 2;
	}
	HeaderParser hp;
	if (!hp.parse(QString::fromUtf8(in.readAll()))) {
		err << "cannot parse: " << header << " error: " << hp.lastError() << endl;
		return 3;
	}
	in.close();

	QFile out(dest);
	if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		err << "cannot write file: " << dest << endl;
		return 4;
	}

	return genSerialize(header, hp, out);
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	QStringList args = app.arguments();
	args.removeFirst();
	if (args.isEmpty()) return usage();

	if (args[0] == "serialize" && args.size() == 3) return serialize(args[1], args[2]);

	return usage();
}
