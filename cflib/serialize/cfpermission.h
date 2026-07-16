/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::serialize {

class CFPermission
{
public:
    CFPermission(const char * name, const char * description = nullptr);

    static List<String> all();
    static CFPermission * lookup(const String & name);
    static void assignIds(const Map<String, uint64> & permissionIds);

public:
    const String name;
    const String description;
    uint32       id = 0;

private:
    static Map<String, CFPermission *> & registry();
};

}
