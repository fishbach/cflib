/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
