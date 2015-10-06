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

	function css(name, value, priority)
	{
		if (value === undefined) return this.style.getPropertyValue(name);
		else                            this.style.setProperty(name, value, !priority ? null : priority /* IE 11 bug */);
	}

	function style(styles)
	{
		if (styles === undefined) return this.style.cssText;
		else                             this.style.cssText = styles;
	}

	function append(el)
	{
		if (el instanceof Fn) el.appendTo(this);
		else                  this.appendChild(el);
	}

	function appendTo(el)
	{
		if (el instanceof Fn) el.append(this);
		else                  el.appendChild(this);
	}

	function addClass(name)
	{
		if (this.className.search(new RegExp('(^|\\s)' + name + '(\\s|$)')) != -1) return;
		this.className += ' ' + name;
	}

	function removeClass(name)
	{
		this.className = this.className.replace(new RegExp('(^|\\s+)' + name + '(\\s+|$)', 'g'), ' ');
	}

	function getValue(value)
	{
		if (value === undefined) return this.value;
		else                            this.value = value;
	}

	function closest(sel)
	{
		var e = this.el.parentNode;
		while (e && e != document) {
			var a = e.parentNode.querySelectorAll(sel);
			var i = a.length;
			while (i--) if (a[i] == e) return new Fn(e);
			e = e.parentNode;
		}
		return null;
	}

	function getWidth()
	{
		if (this.el == document) return document.body.clientWidth;
		return this.el.offsetWidth;
	}

	function getHeight()
	{
		if (this.el == document) return document.body.clientHeight;
		return this.el.offsetHeight;
	}

	function data(name, value)
	{
		if (value === undefined) return this.el.getAttribute('data-' + name);
		else                            this.el.setAttribute('data-' + name, value);
	}

	function checkEvent(e, sel, scope)
	{
		var el = e.target;
		var p = el.parentNode;
		while (el != scope && p) {
			var a = p.querySelectorAll(sel);
			var i = a.length;
			while (i--) if (a[i] == el) return el;
			el = p;
			p = el.parentNode;
		}
		return null;
	}

	function Fn(el) { this.el = el; }

	// ========================================================================

	var $ = function(sel, scope) {
		var el =
			/*jshint smarttabs:true */
			typeof sel != 'string' ? sel :
			!scope                 ? document.querySelector(sel) :
			scope instanceof Fn    ? scope.el.querySelector(sel) :
			                         scope.querySelector(sel);
		return el ? new Fn(el) : null;
	};

	$.all = function(sel, scope) {
		var els =
			/*jshint smarttabs:true */
			!scope                 ? document.querySelectorAll(sel) :
			scope instanceof Fn    ? scope.el.querySelectorAll(sel) :
			                         scope.querySelectorAll(sel);
		return els.length > 0 ? new Fn(els) : null;
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
	};

	$.checkEvent = function(sel, func) {
		return function(e) {
			var el = checkEvent(e, sel, this);
			if (!el) return;
			if (func.call(el, e) !== false) e.stopPropagation();
		};
	};

	$.extend = function(obj1, obj2) { for (var key in obj2) obj1[key] = obj2[key]; };

	// ------------------------------------------------------------------------

	$.fn = Fn.prototype = {

		count: function() { return !this.el ? 0 : Array.isArray(this.el) ? this.el.length : 1; },
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
		append      : function(el)                    { return this.each(append, arguments); },
		appendTo    : function(el)                    { return this.each(appendTo, arguments); },
		addClass    : function(name)                  { return this.each(addClass, arguments); },
		removeClass : function(name)                  { return this.each(removeClass, arguments); },
		toggleClass : function(name, state)           { return this.each(state ? addClass : removeClass, arguments); },
		val         : function(value)                 { return this.each(getValue, arguments); },

		width       : getWidth,
		height      : getHeight,
		offset      : function()                      { return this.el.getBoundingClientRect(); },
		data        : data,

		eq          : function(id)                    { return $(this.el[id]); },
		closest     : closest,
		next        : function()                      { return $(this.el.nextElementSibling    ); },
		prev        : function()                      { return $(this.el.previousElementSibling); }
	};

	return $;
});
