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

	var clId            = null;
	var isFirst         = true;
	var requestActive   = false;
	var waitingRequests = [];
	var ws              = null;

	function getClId()
	{
		if (clId !== null) return clId;
		clId = util.fromBase64(c);
		var c = storage.get('clId');
		if (!c) return null;
		clId = util.fromBase64(c);
		return clId;
	}

	function setClId(newId)
	{
		clId = newId;
		storage.set('clId', util.toBase64(clId), true);
	}

	function wsOpen(e)
	{
		console.log("ws open: ", e);
	}

	function wsClose(e)
	{
		ws = null;
		setTimeout(rmi.start, 5000);
	}

	function newMessage(e)
	{
		console.log("ws: ", e.data);
	}

	// ========================================================================

	var RMI = function() {};
	var rmi = new RMI();

	rmi.ev = {
		loading:      new EV(rmi, "loading"),		// bool isLoading
		identityLost: new EV(rmi, "identityLost"),
		newMessage:   new EV(rmi, "newMessage")		// (string / arraybuffer) data
	};

	rmi.start = function() {
		if (ws) ws.close();
		var loc = window.location;
		ws = new WebSocket((loc.protocol == 'https:' ? 'wss://' : 'ws://') + loc.host + '/ws');
		ws.binaryType = 'arraybuffer';
		ws.onmessage = newMessage;
		ws.onopen = wsOpen;
		ws.onclose = wsClose;
	};

	return rmi;
});
