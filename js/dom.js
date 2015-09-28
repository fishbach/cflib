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

define(function() {

	function html(code) { this.innerHTML = code; }
	function remove() { this.parentNode.removeChild(this); }
	function css(name, value, priority) {
		if (value !== undefined) this.style.setProperty(name, value, priority);
		else                     return this.style.getPropertyValue(name);
	}

	function Fn(el) { this.el = el; }

	// ========================================================================

	var $ = function(sel, scope) {
		return new Fn(typeof sel != 'string' ? sel :
			scope ? scope.querySelector(sel) :
			document.querySelector(sel));
	};

	$.all = function(sel, scope) {
		return new Fn(scope ? scope.querySelectorAll(sel) : document.querySelectorAll(sel));
	};

	$.start = function(func) {
		var rs = document.readyState;
		if (rs == 'complete' || rs == 'loaded' || rs == 'interactive') {
			func();
		} else {
			document.addEventListener('DOMContentLoaded', func);
		}
	};

	$.each = function(a, func) {
		for (var i = 0, len = a.length ; i < len ; ++i) func(a[i], i);
	};

	$.unfocus = function() { document.activeElement.blur(); };

	// ------------------------------------------------------------------------

	$.fn = Fn.prototype = {

		count: function() { var l = this.el.length; return !l ? 1 : l; },
		each: function(func, params) {
			var els = this.el;
			var rv;
			if (!els.length) rv = func.apply(els, params);
			for (var i = 0, len = els.length ; i < len ; ++i) rv = func.apply(els[i], params);
			return rv === undefined ? this : rv;
		},

		html: function(code) { return this.each(html, arguments); },

		on : function(name, func, capture) { return this.each(EventTarget.addEventListener,    arguments); },
		off: function(name, func)          { return this.each(EventTarget.removeEventListener, arguments); },

		remove: function() { return this.each(remove, arguments); },
		css: function(name, value, priority) { return this.each(css, arguments); }
	};

	return $;
});
