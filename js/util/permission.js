/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

const registry = {};

var Permission = function(name, description) {
    this.name = name;
    this.description = description ? description : null;
    this.id = 0;

    registry[name] = this;
};

Permission.assignIds = function(permissionIds) {
    for (const nameId of permissionIds) {
        if (nameId[0] in registry) registry[nameId[0]].id = nameId[1];
    }
};

export default Permission;
