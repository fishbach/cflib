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

var require, define;
(function(scope) {

var loadedModules = {};
var loadingModules = {};
var defines = {};
var waitingCalls = [];
var basePath = '';
var basePathLen = 0;
var head = null;

function checkWaitingCalls()
{
	var again;
	do {
		again = false;
		var i = waitingCalls.length;
		while (i--) {
			if (waitingCalls[i]()) {
				waitingCalls.splice(i, 1);
				again = true;
			}
		}
	} while (again);

	if (waitingCalls.length === 0) {
		loadingModules = {};
		defines = {};
	}
}

function getModuleName(node)
{
	var path = node.getAttribute('src');
	return path.substr(basePathLen, path.length - basePathLen - 3);
}

function getCurrentModuleName()
{
	if (document.currentScript) return getModuleName(document.currentScript);
	var scripts = document.getElementsByTagName('script');
	var i = scripts.length;
	while (i--) {
		var scr = scripts[i];
		if (scr.readyState == 'interactive') return getModuleName(scr);
	}
	return null;
}

function loadEvent()
{
	this.removeEventListener('load', loadEvent);
	var name = getModuleName(this);
	if (!(name in defines)) {
		loadedModules[name] = null;
		checkWaitingCalls();
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

				var node = document.createElement('script');
				node.addEventListener('load', loadEvent);
				node.async = true;
				node.src = basePath + mod + '.js';
				head.appendChild(node);
			}
		} else if (!needLoad) {
			args.unshift(loadedModules[mod]);
		}
	}
	if (needLoad) return false;

	var rv = func.apply(scope, args);
	if (name) loadedModules[name] = rv;
	return true;
}

function doDefine(name, depends, func)
{
	if (name) defines[name] = true;
	if (func) {
		if (!checkDepends(name, depends, func)) {
			waitingCalls.push(function() { return checkDepends(name, depends, func); });
		} else if (name) checkWaitingCalls();
		return;
	}

	var rv = depends();
	if (name) {
		loadedModules[name] = rv;
		checkWaitingCalls();
	}
}

require = function(depends, func) { doDefine(null, depends, func); };
define  = function(depends, func) { doDefine(getCurrentModuleName(), depends, func); };

(function() {
	var scripts = document.getElementsByTagName('script');
	var i = scripts.length;
	while (i--) {
		var scr = scripts[i];
		var main = scr.getAttribute('data-main');
		if (!main) continue;

		var p = main.lastIndexOf('/');
		if (p != -1) {
			++p;
			basePath = main.substr(0, p);
			basePathLen = basePath.length;
		}
		head = scr.parentNode;

		var node = document.createElement('script');
		node.async = true;
		node.src = main;
		head.appendChild(node);
		return;
	}
	throw new Error('attribute "data-main" missing');
})();

})(this);
