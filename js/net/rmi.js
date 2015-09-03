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
	'cflib/net/ber',
	'cflib/util/ev',
	'cflib/util/storage',
	'cflib/util/util'
], function(ber, EV, storage, util) {

	var isFirst         = true;
	var requestActive   = false;
	var waitingRequests = [];
	var ws              = null;

	function wsOpen(e)
	{
		var id = storage.get('clId');
		if (id) ws.send(ber.makeTLV(1, false, util.fromBase64(id)));
		else    ws.send(ber.makeTLV(1));
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
		console.log(tlv, data);
		switch (tlv[1]) {
			case 1:
				storage.set('clId', util.toBase64(new Uint8Array(data.buffer, tlv[2] + tlv[3], tlv[0])), true);
				rmi.ev.identityReset.fire();
				break;
			default:
				rmi.ev.newMessage.fire(data);
		}
	}

	// ========================================================================

	var RMI = function() {};
	var rmi = new RMI();

	rmi.ev = {
		loading:       new EV(rmi, "loading"),			// bool isLoading
		identityReset: new EV(rmi, "identityReset"),
		newMessage:    new EV(rmi, "newMessage")		// (string / arraybuffer) data
	};

	rmi.start = function() {
		if (ws) return;
		var loc = window.location;
		ws = new WebSocket((loc.protocol == 'https:' ? 'wss://' : 'ws://') + loc.host + '/ws');
		ws.binaryType = 'arraybuffer';
		ws.onmessage = newMessage;
		ws.onopen = wsOpen;
		ws.onclose = wsClose;
	};

	rmi.stop = function() {
		if (!ws) return;
		var cl = ws;
		ws = null;
		cl.close();
	};

	return rmi;
});
