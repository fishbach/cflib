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

Permission::Permission(const String & name, const String & description) :
    name(name),
    description(description)
{
    registry()[this->name] = this;
}

String Permission::getNS() const
{
    ssize_t pos = name.lastIndexOf(".");
    if (pos == -1) return {};
    return name.left(pos).replace(".", "::");
}

String Permission::getNSPath() const
{
    return getNS().replace("::", "/").toLower();
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
    for (const String & perm : registry().keys()) {
        uint64 id = permissionIds.value(perm);
        if (id == 0) logWarn("no id for permission '%1' found", perm);
        else         registry().value(perm)->id = id;
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
