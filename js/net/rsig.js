/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

define([
	'cflib/net/ber',
	'cflib/net/rmi',
	'cflib/util/ev',
	'cflib/util/inherit'
], function(ber, rmi, EV, inherit) {

	var RSig = function(source, name, service, ser) {
		EV.call(this, source, name);
		this.service = service;
		this.ser     = ser;
	};
	inherit.setBase(RSig, EV);

	RSig.prototype.register = function() {
		var s = ber.S().s(this.service).s(this.name).i(true).i(arguments.length);
		var args = Array.prototype.slice.call(arguments);
		args.unshift(s);
		this.ser.apply(this, args);
		rmi.sendAsync(s.box(2));
		return this;
	};

	RSig.prototype.unregister = function() {
		var s = ber.S().s(this.service).s(this.name).i(false).i(arguments.length);
		var args = Array.prototype.slice.call(arguments);
		args.unshift(s);
		this.ser.apply(this, args);
		rmi.sendAsync(s.box(2));
		return this;
	};

	return RSig;
});
