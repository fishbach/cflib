/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include <cflib/crypt/util.h>

#ifdef Q_OS_WIN32
	#include <stdio.h>
#else
	#include <unistd.h>
#endif

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	#ifdef Q_OS_WIN32
		char pwd[256];
		printf("Password: ");
		gets_s(pwd, 256);
	#else
		char * pwd = getpass("Password: ");
	#endif
	QTextStream(stdout) << cflib::crypt::hashPassword(pwd) << endl;

	return 0;
}
