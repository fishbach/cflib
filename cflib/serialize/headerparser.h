/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serializetypeinfo.h>

#include <string>

namespace cflib::serialize {

class HeaderParser
{
public:
    bool parse(const std::string & header);
    std::string lastError() const { return lastError_; }

    bool hasSerializeElements() const { return !classes_.isEmpty(); }
    SerializeTypeInfos classes() const { return classes_; }

private:
    bool getVariables   (const std::string & in, int start, int end, SerializeTypeInfo & cl);
    bool getParameters  (const std::string & in, int start, int end, SerializeVariableTypeInfos & vars);
    bool getFunctions   (const std::string & in, int start, int end, SerializeTypeInfo & cl);
    bool getCfSignals   (const std::string & in, int start, int end, SerializeTypeInfo & cl);
    bool getMembers     (const std::string & in, int start, int end, SerializeTypeInfo & cl, int & state);
    bool getMemberBlocks(const std::string & in, int start, int end, SerializeTypeInfo & cl, int & state);
    bool getClasses     (const std::string & in, int start, int end, SerializeTypeInfo cl);
    bool removeCommentsAndStringContents(std::string & header);

private:
    std::string lastError_;
    SerializeTypeInfos classes_;
};

} // namespace
