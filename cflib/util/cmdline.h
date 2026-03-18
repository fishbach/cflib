/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfbytearray.h>
#include <cflib/base/cfcontainers.h>

namespace cflib { namespace util {

class ArgBase
{
public:
    bool isSet() const { return count_ > 0; }
    CFByteArray value(const CFByteArray & defaultValue = CFByteArray()) const { return values_.empty() ? defaultValue : values_.front(); }
    CFList<CFByteArray> values() const { return values_; }
    cfuint count() const { return count_; }

protected:
    ArgBase(char optionChar, const CFByteArray & optionName, bool hasValue, bool isOptional, bool isRepeatable) :
        optionChar_(optionChar), optionName_(optionName), hasValue_(hasValue), isOptional_(isOptional), isRepeatable_(isRepeatable),
        count_(0) {}

    char optionChar_;
    CFByteArray optionName_;
    bool hasValue_;
    bool isOptional_;
    bool isRepeatable_;
    cfuint count_;
    CFList<CFByteArray> values_;

    friend class CmdLine;
};

class Arg : public ArgBase
{
public:
    Arg(bool isOptional = true, bool isRepeatable = false) :
        ArgBase(0, CFByteArray(), true, isOptional, isRepeatable) {}
};

class Option : public ArgBase
{
public:
    Option(const CFByteArray & optionName, bool hasValue = false,
        bool isOptional = true, bool isRepeatable = false) :
        ArgBase(0, optionName, hasValue, isOptional, isRepeatable) {}
    Option(char optionChar, const CFByteArray & optionName = CFByteArray(), bool hasValue = false,
        bool isOptional = true, bool isRepeatable = false) :
        ArgBase(optionChar, optionName, hasValue, isOptional, isRepeatable) {}
};

class CmdLine
{
public:
    CmdLine(int argc, char *argv[]);
    bool parse();
    CmdLine & operator<<(Arg & arg);
    CmdLine & operator<<(Option & arg);

    CFByteArray executable() const { return executable_; }

private:
    CFList<CFByteArray> rawArgs_;
    CFList<Arg *> args_;
    CFHash<char, Option *> shortOptions_;
    CFHash<CFByteArray, Option *> options_;
    CFList<ArgBase *> nonOptionals_;
    CFByteArray executable_;
};

}}    // namespace
