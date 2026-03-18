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

#include <format>
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using namespace cflib::serialize;
using namespace cflib::util;

namespace {

int showUsage(const CFByteArray & executable)
{
    std::cerr << std::format(
        "Usage: {} [options]\n"
        "Options:\n"
        "  -h, --help   => this help\n"
        "  -x, --hex    => input is hex encoded\n"
        "  -b, --base64 => input is Base64 encoded\n",
        executable.constData());
    return 1;
}


void show(const CFByteArray & data, bool hex, bool)
{
    const CFByteArray rawData =
        hex ? CFByteArray::fromHex(data) :
        data;

    std::cout << std::format("{}\n", printAsn1(rawData).c_str());
}

}

int main(int argc, char *argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help     ('h', "help"  ); cmdLine << help;
    Option hexOpt   ('x', "hex"   ); cmdLine << hexOpt;
    Option base64Opt('b', "base64"); cmdLine << base64Opt;
    if (!cmdLine.parse() || help.isSet() || (hexOpt.isSet() && base64Opt.isSet())) return showUsage(cmdLine.executable());

    const CFByteArray buf(0x10000, '\0');
    CFByteArray data;

    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 250000;

    fd_set fds;
    FD_ZERO(&fds);

    while (true) {
        FD_SET(0, &fds);
        int retval = select(1, &fds, NULL, NULL, &tv);
        if (retval == -1) {
            std::cerr << "cannot read from stdin\n";
            return 1;
        }
        if (retval > 0) {
            cfint64 count = read(0, (void *)buf.constData(), buf.size());
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
