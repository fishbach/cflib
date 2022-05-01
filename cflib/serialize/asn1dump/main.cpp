/* Copyright (C) 2013-2018 Christian Fischbach <cf@cflib.de>
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

#include <cflib/serialize/asn1dump.h>
#include <cflib/util/cmdline.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using namespace cflib::serialize;
using namespace cflib::util;

namespace {

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Usage: " << executable << " [options]"     << endl
		<< "Options:"                                  << endl
		<< "  -h, --help   => this help"               << endl
		<< "  -x, --hex    => input is hex encoded"    << endl
		<< "  -b, --base64 => input is Base64 encoded" << endl;
	return 1;
}


void show(const QByteArray & data, bool hex, bool base64)
{
	const QByteArray rawData =
		hex    ? QByteArray::fromHex(data)    :
		base64 ? QByteArray::fromBase64(data) :
		data;

	static QTextStream out(stdout);

	out << printAsn1(rawData) << endl;
}

}

int main(int argc, char *argv[])
{
	CmdLine cmdLine(argc, argv);
	Option help     ('h', "help"  ); cmdLine << help;
	Option hexOpt   ('x', "hex"   ); cmdLine << hexOpt;
	Option base64Opt('b', "base64"); cmdLine << base64Opt;
	if (!cmdLine.parse() || help.isSet() || (hexOpt.isSet() && base64Opt.isSet())) return showUsage(cmdLine.executable());

	QCoreApplication a(argc, argv);

	const QByteArray buf(0x10000, '\0');
	QByteArray data;

	struct timeval tv;
	tv.tv_sec  = 0;
	tv.tv_usec = 250000;

	fd_set fds;
	FD_ZERO(&fds);

	forever {
		FD_SET(0, &fds);
		int retval = select(1, &fds, NULL, NULL, &tv);
		if (retval == -1) {
			QTextStream(stderr) << "cannot read from stdin" << endl;
			return 1;
		}
		if (retval > 0) {
			qint64 count = read(0, (void *)buf.constData(), buf.size());
			if (count == 0) {
				// eof
				if (!data.isEmpty()) show(data, hexOpt.isSet(), base64Opt.isSet());
				break;
			}
			data.append(buf.constData(), count);
		} else if (!data.isEmpty()) {
			// timeout
			show(data, hexOpt.isSet(), base64Opt.isSet());
			data.clear();
		}
	}

	return 0;
}
