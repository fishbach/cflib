/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

define([
	'cflib/dom',
	'cflib/net/ber',
	'cflib/util/ev',
	'cflib/util/storage',
	'cflib/util/util'
], function($, ber, EV, storage, util) {

	var requestActive   = false;
	var requestCallback = null;
	var waitingRequests = [];
	var ws              = null;
	var msgHandlers     = {};
	var rsigHandlers    = {};
	var rsigId          = 0;
	var waitingRSig     = [];
	var waitingAsync    = [];
	var id              = null;

	function checkWaitingRequests()
	{
		if (waitingRequests.length > 0) {
			var wr = waitingRequests.shift();
			requestCallback = wr[1];
			ws.send(wr[0]);
		} else {
			requestCallback = null;
			requestActive = false;
			rmi.ev.loading.fire(false);

			while (waitingRSig.length > 0) {
				var rsig = waitingRSig.shift();
				rsig[0](rsig[1]);
			}
		}
	}

	function getId()
	{
		if (!id) {
			id = storage.get('clId');
			if (id) id = util.fromBase64(id);
		}
		return id;
	}

	function wsOpen(e)
	{
		var id = getId();
		if (id) ws.send(ber.makeTLV(1, false, id));
		else    ws.send(ber.makeTLV(1));
		$.each(waitingAsync, function(data) { ws.send(data); });
		waitingAsync = [];
		checkWaitingRequests();
	}

	function wsClose(e)
	{
		if (ws) {
			ws = null;
			setTimeout(rmi.start, 5000);
		}
	}

	function newMessage(e)
	{
		var data = e.data;
		if (!(data instanceof ArrayBuffer)) {
			rmi.ev.newMessage.fire(data);
			return;
		}
		data = new Uint8Array(data);
		// console.log("msg: ", [...data].map(x => x.toString(16).padStart(2, '0')).join(''));
		var tlv = ber.decodeTLV(data, 0, data.length);
		var tagNo = tlv[1];
		var value = new Uint8Array(data.buffer, tlv[2] + tlv[3], tlv[0]);
		switch (tagNo) {
			case 1:
				id = new Uint8Array(value.length);
				id.set(value);
				storage.set('clId', util.toBase64(id), true);
				rmi.ev.identityReset.fire();
				return;
			case 2:
				if (requestCallback) requestCallback(value);
				checkWaitingRequests();
				return;
			case 3:
				var deser = ber.D(value);
				var func = rsigHandlers[deser.i()];
				if (func) {
					deser = ber.D(deser.a());
					if (waitingRequests.length > 0) waitingRSig.push([func, deser]);
					else                            func(deser);
				}
				return;
			default:
				if (tagNo in msgHandlers) msgHandlers[tagNo](value);
				else                      rmi.ev.newMessage.fire(data);
		}
	}

	// ========================================================================

	var RMI = function() {};
	var rmi = new RMI();

	rmi.ev = {
		loading:       new EV(rmi, 'loading'),			// bool isLoading
		identityReset: new EV(rmi, 'identityReset'),
		newMessage:    new EV(rmi, 'newMessage')		// (string / arraybuffer) data
	};

	rmi.start = function() {
		if (ws) return;
		var loc = window.location;
		ws = new WebSocket((loc.protocol == 'https:' ? 'wss://' : 'ws://') + loc.host + '/ws');
		ws.binaryType = 'arraybuffer';
		ws.onmessage = newMessage;
		ws.onopen = wsOpen;
		ws.onclose = wsClose;
		rmi.ev.loading.fire(true);
		requestActive = true;
	};

	rmi.isRunning = function() { return !!ws; };

	rmi.stop = function() {
		if (!ws) return;
		var cl = ws;
		ws = null;
		cl.close();
	};

	rmi.id = getId;

	rmi.sendAsync = function(data) {
		if (ws.readyState != 1) waitingAsync.push(data);
		else                    ws.send(data);
	};

	rmi.sendRequest = function(data, callback) {
		if (requestActive) waitingRequests.push([data, callback]);
		else {
			rmi.ev.loading.fire(true);
			requestActive = true;
			requestCallback = callback;
			ws.send(data);
		}
	};

	rmi.register     = function(tagNo, func) { msgHandlers [tagNo] = func; };
	rmi.registerRSig = function(func) { rsigHandlers[++rsigId] = func; return rsigId; };

	// ========================================================================

	rmi.store = function(name, obj, permanent) {
		if (!obj) storage.remove(name);
		else      storage.set(name, util.toBase64(ber.S().o(obj).data()), permanent);
	};

	rmi.restore = function(name, Type) {
		try {
			var data = storage.get(name);
			if (!data) return null;
			return new Type(ber.D(util.fromBase64(data)).a());
		} catch (e) {}
		return null;
	};

	rmi.resetStorage = function(keepItems) {
		if (!keepItems) keepItems = [];
		else if (!Array.isArray(keepItems)) keepItems = [keepItems];
		keepItems.push('clId');
		storage.clear(keepItems);
	};

	return rmi;
});
