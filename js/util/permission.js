/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

var Permission = function(name, description) {
    this.name = name;
    this.description = description ? description : null;
    this.id = 0;

    Permission.registry[name] = this;
};

Permission.registry = {};

export default Permission;
