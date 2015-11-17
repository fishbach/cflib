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

	var Storage = function() {};
	var storage = new Storage();

	storage.get = function(name) {
		var rv = window.localStorage.getItem(name);
		if (rv === null) return window.sessionStorage.getItem(name);
		return rv;
	};

	storage.set = function(name, val, permanent) {
		if (permanent) window.localStorage.setItem(name, val);
		else           window.sessionStorage.setItem(name, val);
	};

	storage.remove = function(name) {
		window.localStorage.removeItem(name);
		window.sessionStorage.removeItem(name);
	};

	storage.clear = function(keepItems) {
		var i;
		var keep = [];
		if (keepItems) {
			if (!keepItems.length) keepItems = [keepItems];
			i = keepItems.length;
			while (i--) {
				var name = keepItems[i];
				var rv = window.localStorage.getItem(name);
				if (rv !== null) keep.push([true, name, rv]);
				else {
					rv = window.sessionStorage.getItem(name);
					if (rv !== null) keep.push([false, name, rv]);
				}
			}
		}
		window.localStorage.clear();
		window.sessionStorage.clear();
		i = keep.length;
		while (i--) {
			var el = keep[i];
			if (el[0]) window.localStorage  .setItem(el[1], el[2]);
			else       window.sessionStorage.setItem(el[1], el[2]);
		}
	};

	return storage;
});
