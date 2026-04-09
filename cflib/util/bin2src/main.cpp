#include <cflib/util/cmdline.h>

#include <iostream>

using namespace cflib::util;

namespace {

auto && err = std::cerr;

int showUsage(const ByteArray & executable)
{
    err
        << "Usage: " << executable.toStdString() << " <binary file> <source file>" << std::endl
        << "Options:"                                                              << std::endl
        << "  -h, --help => this help"                                             << std::endl;
    return 1;
}

}

int main(int argc, char *argv[])
{
    CmdLine cmdLine(argc, argv);
    Option help  ('h', "help"); cmdLine << help;
    Arg    binArg(false      ); cmdLine << binArg;
    Arg    srcArg(false      ); cmdLine << srcArg;
    if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

    ByteArray binData;
    {
        File f(binArg.value());
        if (!f.open(File::ReadOnly)) {
            err << "cannot open " << binArg.value().toStdString() << " for reading." << std::endl;
            return 1;
        }
        binData = f.readAll().toHex();
    }

    File src(srcArg.value());
    if (!src.open(File::WriteOnly | File::Truncate)) {
        err << "cannot open " << srcArg.value().toStdString() << " for writing." << std::endl;
        return 2;
    }

    src.write(
        "#include <cflib/base.h>\n"
        "namespace { const uint8 data[] = {\n"
    );
    size_t i = 0;
    while (i < binData.size()) {
        if (i == 0)           src.write("    0x");
        else if (i % 32 == 0) src.write(",\n    0x");
        else                  src.write(", 0x");
        src.write(&binData[i], 2);
        i += 2;
    }
    src.write(
        "\n"
        "}; }\n"
        "CF_STATIC_EXEC({ File::registerData(\"" + binArg.value() + "\", data, sizeof(data)); })\n"
    );

    return 0;
}
