/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

define([
	'cflib/util/ev'
], function(EV) {

	var loc  = window.location;
	var hist = window.history;
	var usePush = hist && hist.pushState;
	var prev = loc.pathname;

	function getHashPath()
	{
		var hash = loc.hash.substr(1);
		if (loc.pathname == '/') return '/' + hash;
		if (!hash)               return loc.pathname;
		else                     return loc.pathname + '/' + hash;
	}

	function urlPath()
	{
		if (usePush) return loc.pathname;
		else         return getHashPath();
	}

	var current = urlPath();

	function urlChangeEvent()
	{
		var newPath = urlPath();
		if (newPath === current) return;
		prev = current;
		current = newPath;
		url.ev.changed.fire(prev, current);
	}

	if (usePush) {
		window.onpopstate = urlChangeEvent;
	} else {
		window.onhashchange = urlChangeEvent;
	}

	// ========================================================================

	var URL = function() {};
	var url = new URL();

	url.ev = {
		changed: new EV(url, 'changed')	// parameter: (from, to)
	};

	url.prev       = function() { return prev; };
	url.current    = function() { return current;  };
	url.startsWith = function(start) { return (current.indexOf(start) === 0); };
	url.endsWith   = function(end)   { return (current.lastIndexOf(end) == (current.length - end.length)); };

	url.goTo = function(path, noHistory, noEvent) {
		if (path === current) return;

		prev = current;
		current = path;

		if (!usePush) {
			if      (loc.pathname == '/')                       loc.hash = current.substr(1);
			else if (current == loc.pathname)                   loc.hash = '';
			else if (current.indexOf(loc.pathname + '/') === 0) loc.hash = current.substr(loc.pathname.length + 1);
			else loc.href = loc.protocol + '//' + loc.host + '/#' + current.substr(1);
		} else if (noHistory) {
			hist.replaceState({}, document.title, current);
		} else {
			hist.pushState({}, document.title, current);
		}

		if (!noEvent) url.ev.changed.fire(prev, current);
	};

	return url;
});
