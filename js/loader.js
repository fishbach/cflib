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

var require, define, mod;
(function(scope) {

var loadedModules = {};
var loadingModules = {};
var loadingCount = 0;
var defines = {};
var waitingCalls = [];
var basePath = '';
var query = '';
var head = null;
var needName = null;

function setModule(name, value)
{
	loadedModules[name] = value;

	var last = mod;
	var parts = name.split('/');
	var len = parts.length - 1;
	for (var i = 0 ; i < len ; ++i) {
		var p = parts[i];
		if (!(p in last)) last = last[p] = {};
		else              last = last[p];
	}
	last[parts[len]] = value;
}

function checkWaitingCalls()
{
	var again;
	do {
		again = false;
		var i = waitingCalls.length;
		while (i--) {
			if (checkDepends.apply(this, waitingCalls[i])) {
				waitingCalls.splice(i, 1);
				again = true;
			}
		}
	} while (again);

	if (waitingCalls.length === 0) loadingModules = {};
}

function getModuleName(node)
{
	var path = node.getAttribute('src');
	return path.substr(basePath.length, path.length - basePath.length - 3 - query.length);
}

function getCurrentModuleName(func)
{
	if (document.currentScript) func(getModuleName(document.currentScript));
	else                        needName = func;
}

function getLoop(chain, deps)
{
	var last = chain[chain.length - 1];
	for (var d in deps[last]) {
		for (var i = 0, len = chain.length ; i < len ; ++i) {
			if (d == chain[i]) return chain.slice(i).concat(d);
		}
		if (d in deps) {
			var rv = getLoop(chain.concat(d), deps);
			if (rv) return rv;
		}
	}
	return null;
}

function loadEvent()
{
	this.removeEventListener('load', loadEvent);

	var name = getModuleName(this);
	if (needName) {
		needName(name);
		needName = null;
	}

	if (!(name in defines)) {
		loadedModules[name] = null;
		checkWaitingCalls();
	} else {
		delete defines[name];
	}

	if (--loadingCount === 0 && waitingCalls.length > 0) {
		var deps = {};
		var first = null;
		for (var i = 0, len = waitingCalls.length ; i < len ; ++i) {
			var c = waitingCalls[i];
			var from = c[0];
			if (!from) continue;
			if (!first) first = from;
			var o = deps[from] = {};
			var to = c[1];
			for (var j = 0, jLen = to.length ; j < jLen ; ++j) o[to[j]] = true;
		}
		throw new Error('dependency loop detected: ' + getLoop([first], deps).join(' -> '));
	}
}

function checkDepends(name, depends, func)
{
	var needLoad = false;
	var args = [];
	var i = depends.length;
	while (i--) {
		var mod = depends[i];
		if (!(mod in loadedModules)) {
			needLoad = true;

			if (!(mod in loadingModules)) {
				loadingModules[mod] = true;

				++loadingCount;
				var node = document.createElement('script');
				node.addEventListener('load', loadEvent);
				node.async = true;
				node.src = basePath + mod + '.js' + query;
				head.appendChild(node);
			}
		} else if (!needLoad) {
			args.unshift(loadedModules[mod]);
		}
	}
	if (needLoad) return false;

	var rv = func.apply(scope, args);
	if (name) setModule(name, rv);
	return true;
}

function doDefine(name, depends, func)
{
	if (name) defines[name] = true;
	if (func) {
		if (!checkDepends(name, depends, func)) {
			waitingCalls.push(arguments);
		} else {
			if (name) checkWaitingCalls();
		}
	} else {
		var rv = depends();
		if (name) {
			setModule(name, rv);
			checkWaitingCalls();
		}
	}
}

require = function(depends, func) { doDefine(null, depends, func); };
define  = function(depends, func) { getCurrentModuleName(function(name) { doDefine(name, depends, func); }); };
mod     = {};

(function() {
	var scripts = document.getElementsByTagName('script');
	var i = scripts.length;
	while (i--) {
		var scr = scripts[i];
		var main = scr.getAttribute('data-main');
		if (!main) continue;

		// basePath
		var p = main.lastIndexOf('/');
		if (p != -1) basePath = main.substr(0, p + 1);

		// query
		query = scr.getAttribute('src') || '';
		p = query.indexOf('?');
		query = p == -1 ? '' : query.substr(p);

		// head
		head = scr.parentNode;

		var node = document.createElement('script');
		node.async = true;
		node.src = main + query;
		head.appendChild(node);
		return;
	}
	throw new Error('attribute "data-main" missing');
})();

})(this);
