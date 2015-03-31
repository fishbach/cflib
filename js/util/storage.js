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

	var needCookie;
	try {
		needCookie = !window.sessionStorage || !window.localStorage;
		if (!needCookie) {
			var key = 'test';
			var val = '[],Bla123';
			window.localStorage  .setItem(key, val);
			window.sessionStorage.setItem(key, val);
			needCookie = window.localStorage.getItem(key) != val || window.sessionStorage.getItem(key) != val;
			window.localStorage  .removeItem(key);
			window.sessionStorage.removeItem(key);
		}
	} catch (e) {
		needCookie = true;
	}

	var cookieRE = /^\s*(.*)=(.*)$/;
	var cookieDoc = null;
	var cookiePath = null;

	function initCookie(cookieDocUrl, readyCallback)
	{
		cookiePath = cookieDocUrl.substr(0, cookieDocUrl.lastIndexOf('/'));
		if (!cookiePath) cookiePath = '/';
		$('<iframe>')
			.hide()
			.attr('src', cookieDocUrl)
			.load(function() {
				cookieDoc = this.contentWindow.document;
				readyCallback();
			})
			.appendTo('body');
	}

	function getCookie(name)
	{
		var parts = cookieDoc.cookie.split(';');
		var i = 0, len = parts.length;
		while (i < len) {
			var keyVal = cookieRE.exec(parts[i++]);
			if (keyVal && keyVal[1] === name) return decodeURIComponent(keyVal[2]);
		}
		return null;
	}

	function setCookie(name, val, daysValid)
	{
		var expires = '';
		if (daysValid) {
			expires = '; expires=' +
				new Date(new Date().getTime() + daysValid * 86400000).toUTCString();
		}
		cookieDoc.cookie = name + '=' + encodeURIComponent(val) + '; path=' + cookiePath + expires;
	}

	// ========================================================================

	var Storage = function() {};
	var storage = new Storage();

	storage.init = function(cookieDocUrl, readyCallback) {
		if (needCookie) initCookie(cookieDocUrl, readyCallback);
		else readyCallback();
	};

	storage.get = function(name) {
		if (needCookie) return getCookie(name);
		var rv = window.localStorage.getItem(name);
		if (rv === null) return window.sessionStorage.getItem(name);
		return rv;
	};

	storage.set = function(name, val, permanent) {
		if (needCookie)     setCookie(name, val, permanent ? 28 : null);
		else if (permanent) window.localStorage.setItem(name, val);
		else                window.sessionStorage.setItem(name, val);
	};

	storage.remove = function(name) {
		if (needCookie) setCookie(name, '', -1);
		else {
			window.localStorage.removeItem(name);
			window.sessionStorage.removeItem(name);
		}
	};

	storage.clear = function() {
		if (needCookie) {
			var parts = cookieDoc.cookie.split(';');
			var i = 0, len = parts.length;
			while (i < len) {
				var keyVal = cookieRE.exec(parts[i++]);
				if (keyVal) setCookie(keyVal[1], '', -1);
			}
		} else {
			window.localStorage.clear();
			window.sessionStorage.clear();
		}
	};

	return storage;
});
