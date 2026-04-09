#include "file.h"

#include <cflib/base/container.h>

namespace cflib::base {

namespace {

struct Data {
    const uint8 * data = nullptr;
    size_t size = 0;
};
using DataRegistry = Hash<String, Data>;
CF_GLOBAL_STATIC(DataRegistry, dataRegistry)

}

bool File::open(int mode)
{
    if (fp_) return false;

    if (path_.startsWith(":/")) return (mode & WriteOnly) == 0;

    const char * m;
    if ((mode & Append))                              m = "ab";
    else if ((mode & Truncate) && (mode & WriteOnly)) m = "wb";
    else if (mode & WriteOnly)                        m = "wb";
    else                                              m = "rb";
    fp_ = fopen(path_.c_str(), m);
    return fp_ != nullptr;
}

ByteArray File::readAll()
{
    if (path_.startsWith(":/")) {
        Data d = dataRegistry().value(path_.mid(2));
        if (d.size == 0) return ByteArray();
        return ByteArray(d.data, d.size);
    }

    if (!fp_) return ByteArray();
    ByteArray result;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp_)) > 0)
        result.append(buf, n);
    return result;
}

void File::registerData(const String & file, const uint8 * data, size_t size)
{
    dataRegistry()[file] = { data, size };
}

} // namespace
