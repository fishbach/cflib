/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

	function html   (code) { this.innerHTML   = code; }
	function setText(text) { this.textContent = text; }

	function addEventListener   () { this.addEventListener   .apply(this, arguments); }
	function removeEventListener() { this.removeEventListener.apply(this, arguments); }

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

	function insertBeforeMe(el)
	{
		if (el instanceof Fn) el.insertMeBefore(this);
		else                  this.parentNode.insertBefore(el, this);
	}

	function insertMeBefore(el)
	{
		if (el instanceof Fn) el.insertBeforeMe(this);
		else                  el.parentNode.insertBefore(this, el);
	}

	function insertAfterMe(el)
	{
		if (el instanceof Fn) el.insertMeAfter(this);
		else                  this.parentNode.insertBefore(el, this.nextSibling);
	}

	function insertMeAfter(el)
	{
		if (el instanceof Fn) el.insertAfterMe(this);
		else                  el.parentNode.insertBefore(this, el.nextSibling);
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

	function attr(name, value)
	{
		if (value === undefined) return this.getAttribute(name);
		else                            this.setAttribute(name, value);
	}

	function prop(name, active)
	{
		if (active === undefined) return this.getAttribute(name) !== null;
		else if (active)                 this.setAttribute(name, '');
		else                             this.removeAttribute(name);
	}

	function closest(el, sel)
	{
		var e = el.parentNode;
		var chain = [el];
		while (e && e != document) {
			var a = e.querySelectorAll(sel);
			var i = a.length;
			while (i--) {
				var ai = a[i];
				var j = chain.length;
				while (j--) if (ai == chain[j]) return ai;
			}
			chain.push(e);
			e = e.parentNode;
		}
		return null;
	}

	function closestAll(sel)
	{
		var els = this.els;
		var el;

		if (!els) {
			el = closest(this.el, sel);
			return el ? new Fn(el) : null;
		}

		var rv = new Fn();
		rv.els = [];
		for (var i = 0, len = els.length ; i < len ; ++i) {
			el = closest(els[i], sel);
			if (el) rv.els.push(el);
		}
		return rv;
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
		return attr.call(this.el, 'data-' + name, value);
	}

	function index(testFunc)
	{
		var els = this.els;
		for (var i = 0, len = els.length ; i < len ; ++i) if (testFunc.call(els[i])) return i;
		return -1;
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

	var $ = function(sel, subSel, scope) {
		if (typeof subSel == 'string') return $(sel + ' ' + subSel, scope).closest(sel);
		var el =
			/*jshint smarttabs:true */
			typeof sel != 'string' ? sel :
			!subSel                ? document.querySelector(sel) :
			subSel instanceof Fn   ? subSel.el.querySelector(sel) :
			                         subSel.querySelector(sel);
		return el ? new Fn(el) : null;
	};

	$.all = function(sel, subSel, scope) {
		if (typeof subSel == 'string') return $.all(sel + ' ' + subSel, scope).closest(sel);
		var rv = new Fn();
		rv.els =
			/*jshint smarttabs:true */
			!subSel              ? document.querySelectorAll(sel) :
			subSel instanceof Fn ? subSel.el.querySelectorAll(sel) :
			                       subSel.querySelectorAll(sel);
		return rv;
	};

	$.create = function(tagNameOrHtml) {
		if (tagNameOrHtml.charAt(0) != '<') return new Fn(document.createElement(tagNameOrHtml));
		var dummy = document.createElement('div');
		dummy.innerHTML = tagNameOrHtml;
		var els = dummy.childNodes;
		if (els.length <= 1) return new Fn(els[0]);
		var rv = new Fn();
		rv.els = Array.prototype.slice.call(els);	// need slice here, otherwise appendChild will change the list
		return rv;
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

	$.unfocus = function() { var e = document.activeElement; if (e) e.blur(); };

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

		count: function() { return this.els ? this.els.length : 1; },
		each: function(func, params) {
			var els = this.els;
			var rv;
			if (!els) rv = func.apply(this.el, params);
			else for (var i = 0, len = els.length ; i < len ; ++i) rv = func.apply(els[i], params);
			return rv === undefined ? this : rv;
		},

		html           : function(code)                  { return this.each(html, arguments); },
		text           : function(text)                  { return this.each(setText, arguments); },
		on             : function(name, func, capture)   { return this.each(addEventListener,    arguments); },
		off            : function(name, func)            { return this.each(removeEventListener, arguments); },
		remove         : function()                      { return this.each(remove, arguments); },
		css            : function(name, value, priority) { return this.each(css, arguments); },
		style          : function(styles)                { return this.each(style, arguments); },
		append         : function(el)                    { return this.each(append, arguments); },
		appendTo       : function(el)                    { return this.each(appendTo, arguments); },
		insertBeforeMe : function(el)                    { return this.each(insertBeforeMe, arguments); },
		insertMeBefore : function(el)                    { return this.each(insertMeBefore, arguments); },
		insertAfterMe  : function(el)                    { return this.each(insertAfterMe, arguments); },
		insertMeAfter  : function(el)                    { return this.each(insertMeAfter, arguments); },
		addClass       : function(name)                  { return this.each(addClass, arguments); },
		removeClass    : function(name)                  { return this.each(removeClass, arguments); },
		toggleClass    : function(name, state)           { return this.each(state ? addClass : removeClass, arguments); },
		val            : function(value)                 { return this.each(getValue, arguments); },
		attr           : function(name, value)           { return this.each(attr, arguments); },
		prop           : function(name, active)          { return this.each(prop, arguments); },

		width          : getWidth,
		height         : getHeight,
		computed       : function(name)                  { return getComputedStyle(this.el, null).getPropertyValue(name); },

		// viewport offset
		offset         : function()                      { return this.el.getBoundingClientRect(); },
		data           : data,
		hasClass       : function(name)                  { return this.el.className.search(new RegExp('(^|\\s)' + name + '(\\s|$)')) != -1; },
		eventIsFrom    : function(sel, event)            { return checkEvent(event, sel, this.el); },

		eq             : function(id)                    { return $(this.els[id]); },
		index          : index,

		closest        : closestAll,
		next           : function()                      { return $(this.el.nextElementSibling    ); },
		prev           : function()                      { return $(this.el.previousElementSibling); },
		parent         : function()                      { return $(this.el.parentNode            ); }
	};

	return $;
});
