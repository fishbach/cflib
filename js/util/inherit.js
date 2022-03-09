/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

define(function() {

	var Inherit = function() {};
	var inherit = new Inherit();

	inherit.Base = function() { inherit.Base.prototype.__init.apply(this, arguments); };
	inherit.Base.prototype.__init = function() {};
	inherit.setBase = function(cl, base) {
		var Prototype = function() {};
		Prototype.prototype = base.prototype;
		cl.prototype = new Prototype();
		cl.__super = base;
	};

	inherit.map = function(list, func) {
		if (!list) return [];
		var rv = [];
		for (var i = 0, l = list.length ; i < l ; ++i) rv.push(func(list[i]));
		return rv;
	};

	return inherit;
});
