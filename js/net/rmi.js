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
	var waitingRSig     = [];
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
				var func = rsigHandlers[deser.s()];
				if (func) {
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

	rmi.sendAsync = function(data) { ws.send(data); };

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
	rmi.registerRSig = function(name,  func) { rsigHandlers[name]  = func; };

	return rmi;
});
