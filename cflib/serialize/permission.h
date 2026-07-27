/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::serialize {

class Permission
{
public:
    Permission(const String & name, const String & description = {});

    String getNS() const;
    String getNSPath() const;

    static StringList all();
    static Permission * lookup(const String & name);
    static void assignIds(const Map<String, uint64> & permissionIds);

public:
    String name;
    String description;
    uint32 id = 0;

private:
    static Map<String, Permission *> & registry();
};

}
