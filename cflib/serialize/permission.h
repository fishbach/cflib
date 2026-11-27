/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::serialize {

class PermissionData
{
public:
    String getNS() const;
    String getNSPath() const;

public:
    String name;
    String description;
};

class Permission : public PermissionData
{
public:
    Permission(const String & name, const String & description = {});

    // registry stores pointer of this
    ~Permission() = default;
    Permission(const Permission & other) = delete;
    Permission(Permission && other) = delete;
    Permission & operator=(const Permission & other) = delete;
    Permission & operator=(Permission && other) = delete;

    static StringList all();
    static Permission * lookup(const String & name);
    static void assignIds(const Map<String, uint64> & permissionIds);
    static Map<String, uint64> getPermissionIds();

public:
    uint32 id = 0;

private:
    static Map<String, Permission *> & registry();
};

}
