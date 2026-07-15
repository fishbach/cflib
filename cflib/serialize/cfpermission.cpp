/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

 #include "cfpermission.h"

namespace cflib::serialize {

CFPermission::CFPermission(const char * name, const char * description) :
    name(name),
    description(description)
{
    registry()[this->name] = this;
}

List<String> CFPermission::all()
{
    return registry().keys();
}

CFPermission * CFPermission::lookup(const String & name)
{
    return registry().value(name);
}

Map<String, CFPermission *> & CFPermission::registry()
{
    static Map<String, CFPermission *> reg;
    return reg;
}

}
