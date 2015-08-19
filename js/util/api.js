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
	'cflib/util/ajax',
	'cflib/util/storage',
	'cflib/util/ev'
], function(Ajax, Storage, EV) {

	var requestActive   = false;
	var waitingRequests = [];
	var clId            = null;
	var webSock         = null;
	var webSockBuf      = [];
	var isFirst         = true;

	function getClId()
	{
		if (clId !== null) return clId;
		clId = Storage.get('clId');
		if (clId) return clId;
		return '';
	}

	function setClId(newId)
	{
		clId = newId;
		Storage.set('clId', clId, true);
		if (webSock) api.initWebSocket();
	}

	function doRMI(service, params, callback, context)
	{
		var ajaxCallback = function(status, content, xhr) {
			if (status != 200) {
				requestActive = false;
				api.ev.loading.fire(false);
				api.ev.error.fire(status, "ajax error");
				if (callback) callback.call(context);
				return;
			}

			// update client Id
			if (content.charAt(0) != '[') {
				setClId(content.substr(0, 40));
				content = content.substr(40);
				if (!isFirst) api.ev.secureTokenLost.fire();
			}
			isFirst = false;

			/*jshint evil: true */
			var data = eval(content);

			if (callback) callback.apply(context, data);

			if (waitingRequests.length > 0) {
				var f = waitingRequests.shift();
				f();
			} else {
				requestActive = false;
				api.ev.loading.fire(false);
			}
		};

		Ajax.request('POST', '/api/rmi/' + service, ajaxCallback, getClId() + api.serialize(params));
	}

	// ========================================================================

	var API = function() {};
	var api = new API();

	api.ev = {
		error:           new EV(api, "error"),		// int status, string msg
		loading:         new EV(api, "loading"),	// bool isLoading
		secureTokenLost: new EV(api, "secureTokenLost"),
		wsReady:         new EV(api, "wsReady"),
		newMessage:      new EV(api, "newMessage")	// (string / arraybuffer) data
	};

	api.rmi = function(service, params, callback, context) {
		if (requestActive) {
			waitingRequests.push(function() { doRMI(service, params, callback, context); });
		} else {
			requestActive = true;
			api.ev.loading.fire(true);
			doRMI(service, params, callback, context);
		}
	};

	api.doAsyncRMI = function(service, params, callback, context) {
		var asyncCallback = function(status, content, xhr) {
			if (!callback) return;
			if (status != 200 || content.charAt(0) != '[') {
				callback.call(context);
			} else {
				/*jshint evil: true */
				var data = eval(content);
				callback.apply(context, data);
			}
		};
		Ajax.request('POST', '/api/rmi/' + service, asyncCallback, getClId() + api.serialize(params));
	};

	api.isRequestActive = function() {
		return requestActive;
	};

	api.getClientId = getClId;

	api.initWebSocket = function() {
		if (webSock) webSock.close();
		var loc = window.location;
		webSock = new WebSocket((loc.protocol == 'https:' ? 'wss://' : 'ws://') + loc.host + '/ws');
		webSock.binaryType = 'arraybuffer';
		webSock.onmessage = function(e) { api.ev.newMessage.fire(e.data); };
		webSock.onopen = function() {
			webSock.send('CFLIB_clientId#' + getClId());
			if (webSockBuf.length > 0) {
				for (var i = 0, len = webSockBuf.length ; i < len ; ++i) webSock.send(webSockBuf[i]);
				webSockBuf = [];
			}
			api.ev.wsReady.fire();
		};
	};

	api.wsSend = function(data) {
		var rs = webSock.readyState;
		if      (rs === 0) webSockBuf.push(data);
		else if (rs ==  1) webSock.send(data);
	};

	// ========================================================================

	api.map = function(arIn, func) {
		if (!(arIn instanceof Array)) return [];
		var arOut = [], i = 0, len = arIn.length;
		while (i < len) arOut.push(func(arIn[i++]));
		return arOut;
	};

	api.map2 = function(arIn, func) {
		if (!(arIn instanceof Array)) return [];
		var arOut = [], i = 0, len = arIn.length;
		while (i < len) {
			arOut.push(func(arIn[i], arIn[i + 1]));
			i += 2;
		}
		return arOut;
	};

	// api.Base is the base class of all automatic generated classes
	api.Base = function() { api.Base.prototype.constructor.apply(this, arguments); };
	api.Base.prototype.constructor = function() {};
	api.setBase = function(cl, base) {
		var Prototype = function() {};
		Prototype.prototype = base.prototype;
		cl.prototype = new Prototype();
		cl.__super = base;
	};

	api.serialize = function(obj) {
		var retval = '';
		if (typeof obj == 'object') {
			if (obj instanceof Array) {
				var lastWasEmpty = false;
				retval += '[';
				var i = 0, len = obj.length, isFirst = true;
				while (i < len) {
					if (isFirst) isFirst = false;
					else retval += ',';
					var subSer = api.serialize(obj[i++]);
					if (!subSer) lastWasEmpty = true;
					else         retval += subSer;
				}
				if (lastWasEmpty) retval += ',';
				retval += ']';
			} else if (obj instanceof api.Base) {
				retval += api.serialize(obj.__memberArray());
			} else if (obj instanceof Date) {
				retval += obj.getTime();
			}
		} else if (typeof obj == 'number') {
			if (obj) retval += obj;
		} else if (typeof obj == 'string') {
			retval += "'" + obj
				.replace(/\\/g, '\\\\')
				.replace(/'/g, "\\'")
				.replace(/\n/g, '\\n')
				.replace(/\r/g, '\\r')
				.replace(/\u2028/g, '\\u2028')
				.replace(/\u2029/g, '\\u2029') +
				"'";
		} else if (typeof obj == 'boolean') {
			if (obj) retval += '1';
		}
		return retval;
	};

	// ========================================================================

	api.store = function(name, obj, permanent) {
		if (!obj) Storage.remove(name);
		else      Storage.set(name, api.serialize(obj), permanent);
	};

	api.restore = function(name, Type) {
		try {
			var data = Storage.get(name);
			if (!data) return null;
			/*jshint evil: true */
			data = eval(data);
			return new Type(data);
		} catch (e) {}
		return null;
	};

	api.resetStorage = function(keepItems) {
		var keep = [];
		if (keepItems) {
			if (!$.isArray(keepItems)) keepItems = [keepItems];
			$.each(keepItems, function(i, name) {
				var val = Storage.get(name);
				if (val) keep.push([name, val]);
			});
		}
		Storage.clear();
		Storage.set('clId', clId, true);
		$.each(keep, function(i, keyVal) { Storage.set(keyVal[0], keyVal[1], true); });
	};

	return api;
});
