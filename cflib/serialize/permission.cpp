/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "permission.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::User)

namespace cflib::serialize {

String PermissionData::getNS() const
{
    ssize_t pos = name.lastIndexOf(".");
    if (pos == -1) return {};
    return name.left(pos).replace(".", "::");
}

String PermissionData::getNSPath() const
{
    return getNS().replace("::", "/").toLower();
}

Permission::Permission(const String & name, const String & description) :
    PermissionData{name, description}
{
    if (registry().contains(name)) {
        logWarn("permission %1 with description %2 instantiated twice", name, description);
    } else {
        registry()[name] = this;
    }
}

StringList Permission::all()
{
    return registry().keys().sorted();
}

Permission * Permission::lookup(const String & name)
{
    return registry().value(name);
}

void Permission::assignIds(const Map<String, uint64> & permissionIds)
{
    for (const auto & [name, perm] : registry()) {
        if (!perm) {
            logWarn("permission %1 has no instance", name);
            continue;
        }
        uint64 id = permissionIds.value(name);
        if (id == 0) logWarn("no id for permission '%1' found", name);
        else         perm->id = id;
    }
}

Map<String, uint64> Permission::getPermissionIds()
{
    Map<String, uint64> rv;
    for (Permission * perm : registry().values()) rv[perm->name] = perm->id;
    return rv;
}

Map<String, Permission *> & Permission::registry()
{
    static Map<String, Permission *> reg;
    return reg;
}

}
