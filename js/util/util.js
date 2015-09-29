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
	'cflib/dom'
], function($) {

	var timeDiff = 0;
	var popupZIndex = 1010;
	var openPopups = [];

	function base64CharToInt(c)
	{
		if (64 < c && c <  91) return c - 65;	// A-Z
		if (96 < c && c < 123) return c - 71;	// a-z
		if (47 < c && c <  58) return c + 4;	// 0-9
		if (c == 43) return 62;					// +
		if (c == 47) return 63;					// /
		return 0;
	}

	var base64Chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
	var Popup = function() {};
	Popup.prototype.onClose = function(callback, context) {
		this.closeFunc     = callback;
		this.closeContext  = context;
	};

	var clipboardDataFunc = null;
	var clipboardBeforecopy = function(e) { e.preventDefault(); };

	// ========================================================================

	var Util = function() {};
	var util = new Util();

	util.makeHash = function(array) {
		var rv = {};
		var i = array.length;
		while (i--) {
			var e = array[i];
			rv[e[0]] = e[1];
		}
		return rv;
	};

	util.simplifyStr = function(str, keepNewlines) {
		if (!keepNewlines) return (str
			.replace(/^\s+|\s+$/g, '')
			.replace(/\s+/g, ' '));
		var rv = '';
		var lines = str.split('\n');
		for (var i = 0, len = lines.length ; i < len ; ++i) {
			if (i > 0) rv += '\n';
			rv += util.simplifyStr(lines[i]);
		}
		return rv;
	};

	util.getInputStr = function(el) {
		return util.simplifyStr(el.value).replace(/[^\w@\._\- ]+/g, '');
	};

	util.dateFromUTC = function(date) {
		return new Date(date.getUTCFullYear(), date.getUTCMonth(), date.getUTCDate());
	};

	util.debounce = function(func, ms) {
		if (!ms) ms = 50;
		var timer = null;
		return function() {
			var scope = this;
			var param = arguments;
			if (timer) clearTimeout(timer);
			timer = setTimeout(function() {
				timer = null;
				func.apply(scope, param);
			}, ms);
		};
	};

	util.throttle = function(func, ms) {
		if (ms === undefined) ms = 50;
		var running = false;
		var last = null;
		return function() {
			if (running || new Date() - last < ms) return;
			running = true;
			func.apply(this, arguments);
			running = false;
			last = new Date();
		};
	};

	util.defer = function(func) {
		return function() {
			var scope = this;
			var param = arguments;
			setTimeout(function() { func.apply(scope, param); }, 0);
		};
	};

	util.removeFromArray = function(array, el) {
		// unfortunately IE 8 does not have 'indexOf' on Arrays
		var i = array.length;
		while (i--) if (array[i] === el) {
			array.splice(i, 1);
			return true;
		}
		return false;
	};

	util.validInputChars = function(str, keepNewlines) {
		str = util.simplifyStr(str, keepNewlines);
		if (/[{}\[\]<>;"\\]/.test(str)) return null;
		return str;
	};

	util.formatInt = function(i, w) {
		var r = '' + i;
		while (r.length < w) r = '0' + r;
		return r;
	};

	util.addThousandsSep = function(i) {
		var r = '' + i;
		var p = r.length - 3;
		while (p > 0) { r = r.substr(0, p) + '.' + r.substr(p); p -= 3; }
		return r;
	};

	util.formatWeekDay = function(date) {
		switch (date.getUTCDay()) {
			case 0: return 'Sun';
			case 1: return 'Mon';
			case 2: return 'Tue';
			case 3: return 'Wed';
			case 4: return 'Thu';
			case 5: return 'Fri';
			case 6: return 'Sat';
		}
	};

	util.isWeekend = function(date) {
		switch (date.getUTCDay()) {
			case 0:
			case 6:  return true;
			default: return false;
		}
	};

	util.formatDateTime = function(date, withSecs, withWeekDay, noNbsp) {
		return (
			(withWeekDay ? util.formatWeekDay(date) + (noNbsp ? ' ' : '&nbsp;') : '') +
			util.formatInt(date.getDate(), 2) + '.' +
			util.formatInt(date.getMonth() + 1, 2) + '.' +
			util.formatInt(date.getFullYear(), 4) + (noNbsp ? ' ' : '&nbsp;') +
			util.formatInt(date.getHours(), 2) + ':' +
			util.formatInt(date.getMinutes(), 2) +
			(withSecs ? ':' + util.formatInt(date.getSeconds(), 2) : '')
		);
	};

	util.formatDayMonth = function(date) {
		return (
			util.formatInt(date.getUTCDate(), 2) + '.' +
			util.formatInt(date.getUTCMonth() + 1, 2) + '.'
		);
	};

	util.endsWith = function(str, part) {
		return (str.lastIndexOf(part) == (str.length - part.length));
	};

	util.dateIsValid = function(date) {
		return (
			Object.prototype.toString.call(date) === "[object Date]" &&
			!isNaN(date.getTime())
		);
	};

	util.numberIsValid = function(n) {
		return (!isNaN(parseFloat(n)) && isFinite(n));
	};

	util.setRefTime = function(date) {
		timeDiff = date.getTime() - new Date().getTime();
	};

	util.now = function(doFormat) {
		if (doFormat) return util.formatDateTime(util.now(), false, true);
		return new Date(new Date().getTime() + timeDiff);
	};

	util.validEmail = function(str) {
		str = util.validInputChars(str);
		if (str === null || !/^[\w.\-_]+@\w[\w.\-]+\.\w+$/.test(str)) return null;
		return str;
	};

	util.setClipboardDefault = function(dataFunc) {
		if (clipboardDataFunc) {
			document.removeEventListener('beforecopy', clipboardBeforecopy);
			document.removeEventListener('copy',       clipboardDataFunc);
			clipboardDataFunc = null;
		}
		if (!dataFunc) return;
		clipboardDataFunc = function(e) {
			var sel = window.getSelection().toString();
			if (!sel || sel.length < 2) {
				e.preventDefault();
				if (e.clipboardData) e.clipboardData.setData('text/plain', dataFunc());
				else                 window.clipboardData.setData('Text',  dataFunc());
			}
		};
		document.addEventListener('beforecopy', clipboardBeforecopy);
		document.addEventListener('copy',       clipboardDataFunc);
	};

	util.toHex = function(n, len) {
		var rv = n.toString(16).toUpperCase();
		if (len) while (rv.length < len) rv = '0' + rv;
		return rv;
	};

	util.setFullscreen = function(el) {
		if (!el) el = document.documentElement;
		if      (el.requestFullscreen      ) el.requestFullscreen();
		else if (el.mozRequestFullScreen   ) el.mozRequestFullScreen();
		else if (el.webkitRequestFullscreen) el.webkitRequestFullscreen();
		else if (el.msRequestFullscreen    ) el.msRequestFullscreen();
	};

	util.getFullscreenEl = function() {
		return (
			document.fullscreenElement       ||
			document.mozFullScreenElement    ||
			document.webkitFullscreenElement ||
			document.msFullscreenElement
		);
	};

	util.addFullscreenEventListener = function(cb) {
		if      ('onfullscreenchange'       in document) document.addEventListener('fullscreenchange',       cb, false);
		else if ('onmozfullscreenchange'    in document) document.addEventListener('mozfullscreenchange',    cb, false);
		else if ('onwebkitfullscreenchange' in document) document.addEventListener('webkitfullscreenchange', cb, false);
	};

	util.setPointerLock = function(el) {
		if (!el) el = document.documentElement;
		if      (el.requestPointerLock      ) el.requestPointerLock();
		else if (el.mozRequestPointerLock   ) el.mozRequestPointerLock();
		else if (el.webkitRequestPointerLock) el.webkitRequestPointerLock();
	};

	util.getPointerLockEl = function() {
		return (
			document.pointerLockElement       ||
			document.mozPointerLockElement    ||
			document.webkitPointerLockElement
		);
	};

	util.addPointerLockEventListener = function(cb) {
		if      ('onpointerlockchange'       in document) document.addEventListener('pointerlockchange',       cb, false);
		else if ('onmozpointerlockchange'    in document) document.addEventListener('mozpointerlockchange',    cb, false);
		else if ('onwebkitpointerlockchange' in document) document.addEventListener('webkitpointerlockchange', cb, false);
	};

	util.fromBase64 = function(base64Str) {
		var inLen = base64Str.indexOf('=');
		if (inLen == -1) inLen = base64Str.length;
		var outLen = (inLen * 3 + 1) >>> 2;
		var rv = new Uint8Array(outLen);
		var val = 0;
		var j = 0;
		for (var i = 0 ; i < inLen ; ++i) {
			var pos = i & 3;
			val |= base64CharToInt(base64Str.charCodeAt(i)) << (18 - 6 * pos);
			if (pos == 3 || i == inLen - 1) {
				rv[j++] = val >>> 16;
				if (j == outLen) break;
				rv[j++] = (val >>> 8) & 255;
				if (j == outLen) break;
				rv[j++] = val & 255;
				val = 0;
			}
		}
		return rv;
	};

	util.toBase64 = function(uint8Array) {
		var rv = '';
		var inLen = uint8Array.length;
		var val = 0;
		for (var i = 0 ; i < inLen ; ++i) {
			var pos = i % 3;
			val |= uint8Array[i] << (16 - 8 * pos);
			if (pos == 2 || i == inLen - 1) {
				rv += base64Chars.charAt(val >>> 18);
				rv += base64Chars.charAt((val >>> 12) & 63);
				if (pos === 0) rv += '==';
				else {
					rv += base64Chars.charAt((val >>> 6) & 63);
					if (pos == 1) rv += '=';
					else          rv += base64Chars.charAt(val & 63);
				}
				val = 0;
			}
		}
		return rv;
	};

	util.createBackdrop = function(darken) {
		var $rv = $
			.create('div')
			.style('position:fixed;top:0;right:0;bottom:0;left:0;z-index:' +
				(++popupZIndex) + ';background-color:#000;opacity:' + (darken ? '0.5' : '0'))
			.appendTo(document.body);
		var remove = $rv.remove;
		$rv.remove = function() { remove.apply(this, arguments); --popupZIndex; };
		return $rv;
	};

	util.showPopup = function($el, x, y, w, darken) {
		var $backdrop = util.createBackdrop(darken);
		var $arrow    = $('div', $el);

		$el.style('visibility:hidden;display:block');
		var ew = $el.width();
		var eh = $el.height();
		var sw = $(document).width();
		var sh = $(document).height();

		// window pos
		var wy = eh >= 44 ? y - 22 : y - eh / 2;
		if (wy < 5) wy = 5;
		else if (wy > sh - eh - 5) wy = sh - eh - 5;

		// arrow pos
		var ay = y - wy;
		if (ay < 8) ay = 8;
		else if (ay > eh - 8) ay = eh - 8;
		$arrow.css('top', ay + 'px');

		var wx;
		if (x + w > sw - ew - 5) {
			wx = x - ew + 5;
			$arrow.toggleClass('arrowLeft', false).toggleClass('arrowRight', true );
		} else {
			wx = x + w - 5;
			$arrow.toggleClass('arrowLeft', true ).toggleClass('arrowRight', false);
		}

		var rv = new Popup();
		var noEv = function(e) { e.stopPropagation(); };
		var escClose = function(e) { if (e.keyCode == 27) rv.close(); };
		rv.close = function() {
			var i = openPopups.length;
			while (i--) if (openPopups[i] == rv) { openPopups.splice(i, 1); break; }

			$el.off('click', closeEv).css('display', 'none');
			$backdrop.off('mousedown', closeEv).remove();
			$arrow.off('click', noEv);
			$('.popupContent', $el).off('click', noEv);
			$(document).off('keydown', escClose);
			popupZIndex -= 2;
			rv.close = function() {};
			if (rv.closeFunc) rv.closeFunc.call(rv.closeContext);
		};

		var closeEv = function(e) {
			e.preventDefault();
			e.stopPropagation();
			rv.close();
		};
		$backdrop.on('mousedown', closeEv);
		$el.on('click', closeEv);
		$arrow.on('click', noEv);
		$('.popupContent', $el).on('click', noEv);

		$.unfocus();
		$el.style('z-index:' + (++popupZIndex) + ';display:block;top:' + wy + 'px;left:' + wx + 'px');

		$(document).on('keydown', escClose);

		openPopups.push(rv);
		return rv;
	};

	util.closeAllPopups = function() {
		while (openPopups.length > 0) openPopups[openPopups.length - 1].close();
	};

	util.preloadImage = function(src, callback, context) {
		if (!Array.isArray(src)) src = [src];
		var count = src.length;
		$.each(src, function(src) {
			var img = new Image();
			var loadFunc = function() {
				img.removeEventListener('load', loadFunc);
				img = null;
				--count;
				if (callback && count === 0) callback.call(context);
			};
			img.addEventListener('load', loadFunc);
			img.src = src;
		});
	};

	return util;
});
