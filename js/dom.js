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

	var spaceRE = /\s+/;

	function html(code) { this.innerHTML = code; }
	function remove() { this.parentNode.removeChild(this); }
	function css(name, value, priority) {
		if (value === undefined) return this.style.getPropertyValue(name);
		else                            this.style.setProperty(name, value, priority);
	}
	function style(styles) {
		if (styles === undefined) return this.style.cssText;
		else                             this.style.cssText = styles;
	}
	function appendTo(el) { el.appendChild(this); }
	function addClass(name) {
		var names = this.className;
		var a = names.split(spaceRE);
		var i = a.length;
		while (i--) if (a[i] == name) return;
		this.className = names + ' ' + name;
	}
	function removeClass(name) {
		var a = this.className.split(spaceRE);
		var i = a.length;
		while (i--) if (a[i] == name) a.splice(i, 1);
		this.className = a.join(' ');
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

	$.create = function(tagName) {
		return new Fn(document.createElement(tagName));
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

	$.scrollPos = function() {
		var el = document.documentElement;
		return [
			(window.pageXOffset || el.scrollLeft) - (el.clientLeft || 0),
			(window.pageYOffset || el.scrollTop ) - (el.clientTop  || 0)];
	}

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

		html        : function(code)                  { return this.each(html, arguments); },
		on          : function(name, func, capture)   { return this.each(this.el.addEventListener,    arguments); },
		off         : function(name, func)            { return this.each(this.el.removeEventListener, arguments); },
		remove      : function()                      { return this.each(remove, arguments); },
		css         : function(name, value, priority) { return this.each(css, arguments); },
		style       : function(styles)                { return this.each(style, arguments); },
		appendTo    : function(el)                    { return this.each(appendTo, arguments); },

		addClass    : function(name)                  { return this.each(addClass, arguments); },
		removeClass : function(name)                  { return this.each(removeClass, arguments); },
		toggleClass : function(name, state)           { return this.each(state ? addClass : removeClass, arguments); },

		width       : function()                      { return this.el.offsetWidth; },
		height      : function()                      { return this.el.offsetHeight; }
	};

	return $;
});
