/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "cmdline.h"

#include <cstring>

namespace cflib::util {

CmdLine::CmdLine(int argc, char *argv[])
{
    for (int i = 0 ; i < argc ; ++i) rawArgs_.push_back(ByteArray(argv[i]));
}

bool CmdLine::parse()
{
    if (rawArgs_.empty()) return false;

    // Extract executable basename
    const ByteArray & fullPath = rawArgs_[0];
    cfsize_t lastSlash = fullPath.indexOf('/');
    cfsize_t pos = lastSlash;
    while (pos >= 0) {
        lastSlash = pos;
        pos = fullPath.indexOf('/', lastSlash + 1);
    }
    executable_ = (lastSlash >= 0) ? fullPath.mid(lastSlash + 1) : fullPath;

    cfsize_t rawIdx = 1;
    int argCount = 0;
    bool parseMoreOptions = true;
    while (rawIdx < (cfsize_t)rawArgs_.size()) {
        const ByteArray & raw = rawArgs_[rawIdx++];

        if (parseMoreOptions && raw.startsWith("--")) {
            if (raw.length() == 2) {
                parseMoreOptions = false;
                continue;
            }
            auto it = options_.find(raw.mid(2));
            if (it == options_.end()) return false;
            Option * opt = it->second;
            if (!opt->isRepeatable_ && opt->count_ > 0) return false;
            opt->count_++;
            if (opt->hasValue_) {
                if (rawIdx >= (cfsize_t)rawArgs_.size()) return false;
                opt->values_.push_back(rawArgs_[rawIdx++]);
            }
        } else if (parseMoreOptions && raw.startsWith("-")) {
            if (raw.length() < 2) return false;
            cfsize_t p = 0;
            while (++p < raw.length()) {
                auto it = shortOptions_.find(raw.at(p));
                if (it == shortOptions_.end()) return false;
                Option * opt = it->second;
                if (!opt->isRepeatable_ && opt->count_ > 0) return false;
                opt->count_++;
                if (opt->hasValue_) {
                    if (p < raw.length() - 1 || rawIdx >= (cfsize_t)rawArgs_.size()) return false;
                    opt->values_.push_back(rawArgs_[rawIdx++]);
                }
            }
        } else {
            if (argCount >= (int)args_.size()) return false;
            Arg * arg = args_[argCount];
            arg->count_++;
            arg->values_.push_back(raw);
            if (!arg->isRepeatable_) ++argCount;
        }
    }

    for (const ArgBase * arg : nonOptionals_) if (!arg->isOptional_ && arg->count_ == 0) return false;

    return true;
}

CmdLine & CmdLine::operator<<(Arg & arg)
{
    args_.push_back(&arg);
    if (!arg.isOptional_) nonOptionals_.push_back(&arg);
    return *this;
}

CmdLine & CmdLine::operator<<(Option & opt)
{
    if (opt.optionChar_ != 0) shortOptions_[opt.optionChar_] = &opt;
    if (!opt.optionName_.isEmpty()) options_[opt.optionName_] = &opt;
    if (!opt.isOptional_) nonOptionals_.push_back(&opt);
    return *this;
}

} // namespace
