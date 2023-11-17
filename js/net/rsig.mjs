/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

import ber     from '../net/ber.mjs';
import rmi     from '../net/rmi.mjs';
import EV      from '../util/ev.mjs';
import inherit from '../util/inherit.mjs';

var RSig = function(source, name, service, rsig, deser) {
	EV.call(this, source, name);
	this.service = service;
	this.rsig    = rsig;
	this.deser   = deser;
	this.id      = 0;
};
inherit.setBase(RSig, EV);

RSig.prototype.register = function() {
	this.id = rmi.registerRSig(this.deser);
	var s = ber.S().s(this.service).s(this.name).i(true).i(this.id);
	rmi.sendAsync(s.box(2));
	return this;
};

RSig.prototype.unregister = function() {
	var s = ber.S().s(this.service).s(this.name).i(false).i(this.id);
	rmi.sendAsync(s.box(2));
	return this;
};

export default RSig;
