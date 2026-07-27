/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/permission.h>
#include <cflib/serialize/serializetypeinfo.h>

namespace cflib::serialize {

class HeaderParser
{
public:
    bool parse(const String & header);
    String lastError() const { return lastError_; }

    bool hasSerializeElements() const { return !(classes_.isEmpty() && permissions_.isEmpty()); }
    SerializeTypeInfos classes() const { return classes_; }
    const List<Permission> & permissions() const { return permissions_; }

private:
    bool getVariables   (const String & in, int start, int end, SerializeTypeInfo & cl);
    bool getParameters  (const String & in, int start, int end, SerializeVariableTypeInfos & vars, const String & ns);
    bool getFunctions   (const String & in, int start, int end, SerializeTypeInfo & cl);
    bool getCfSignals   (const String & in, int start, int end, SerializeTypeInfo & cl);
    bool getMembers     (const String & in, int start, int end, SerializeTypeInfo & cl, int & state);
    bool getMemberBlocks(const String & in, int start, int end, SerializeTypeInfo & cl, int & state);
    bool getPermissions (const String & in, int start, int end, SerializeTypeInfo & cl);
    bool getClasses     (const String & in, int start, int end, SerializeTypeInfo cl);
    bool removeCommentsAndStringContents(String & header);

private:
    String lastError_;
    SerializeTypeInfos classes_;
    List<Permission> permissions_;
};

} // namespace
