/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "cfpermission.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::User)

namespace cflib::serialize {

CFPermission::CFPermission(const String & name, const String & description) :
    name(name),
    description(description)
{
    registry()[this->name] = this;
}

String CFPermission::getNS() const
{
    ssize_t pos = name.lastIndexOf(".");
    if (pos == -1) return {};
    return name.left(pos).replace(".", "::");
}

StringList CFPermission::all()
{
    return registry().keys().sorted();
}

CFPermission * CFPermission::lookup(const String & name)
{
    return registry().value(name);
}

void CFPermission::assignIds(const Map<String, uint64> & permissionIds)
{
    for (const String & perm : registry().keys()) {
        uint64 id = permissionIds.value(perm);
        if (id == 0) logWarn("no id for permission '%1' found", perm);
        else         registry().value(perm)->id = id;
    }
}

Map<String, CFPermission *> & CFPermission::registry()
{
    static Map<String, CFPermission *> reg;
    return reg;
}

}
