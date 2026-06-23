/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

import ber     from '../net/ber.js';
import rmi     from '../net/rmi.js';
import EV      from '../util/ev.js';
import inherit from '../util/inherit.js';

var RSig = function(source, name, service, rsig, deser) {
    EV.call(this, source, name);
    this.service = service;
    this.rsig    = rsig;
    this.deser   = deser;
    this.id      = 0;
};
inherit.setBase(RSig, EV);

RSig.prototype.register = function() {
    rmi.registerRSig(this);
    return this;
};

RSig.prototype.unregister = function() {
    rmi.unregisterRSig(this.id);
};

export default RSig;
