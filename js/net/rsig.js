/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
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
		this.ser(s, arguments);
		rmi.sendAsync(s.box(2));
	};

	RSig.prototype.unregister = function() {
		var s = ber.S().s(this.service).s(this.name).i(false).i(arguments.length);
		this.ser(s, arguments);
		rmi.sendAsync(s.box(2));
	};

	return RSig;
});
