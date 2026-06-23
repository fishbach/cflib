/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

import $       from '../dom.js';
import ber     from '../net/ber.js';
import EV      from '../util/ev.js';
import storage from '../util/storage.js';
import util    from '../util/util.js';

let requestActive   = false;
let requestCallback = null;
let waitingRequests = [];
let ws              = null;
let msgHandlers     = {};
let rsigHandlers    = {};
let rsigId          = 0;
let waitingRSig     = [];
let waitingAsync    = [];
let id              = null;
let lastAlive       = null;
let aliveTimeout    = false;

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
    rmi.ev.connectionOpened.fire();
    Object.values(rsigHandlers).forEach(rsig => sendRsigRegistration(rsig));
    $.each(waitingAsync, function(data) { ws.send(data); });
    waitingAsync = [];
    checkWaitingRequests();
}

function wsClose(e)
{
    if (ws) {
        ws = null;
        setTimeout(rmi.start, 5000);

        rmi.ev.connectionClosed.fire();
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
            let deser = ber.D(value);
            let rsig = rsigHandlers[deser.i()];
            if (rsig) {
                let func = rsig.deser;
                deser = ber.D(deser.a());
                if (requestActive) waitingRSig.push([func, deser]);
                else               func(deser);
            }
            return;
        default:
            if (tagNo in msgHandlers) msgHandlers[tagNo](value);
            else                      rmi.ev.newMessage.fire(data);
    }
}

function sendRsigRegistration(rsig)
{
    rmi.sendAsync(ber.S().s(rsig.service).s(rsig.name).i(true).i(rsig.id).box(2), true);
}

// ========================================================================

var RMI = function() {};
var rmi = new RMI();

rmi.ev = {
    loading:          new EV(rmi, 'loading'),            // bool isLoading
    identityReset:    new EV(rmi, 'identityReset'),
    newMessage:       new EV(rmi, 'newMessage'),        // (string / arraybuffer) data
    connectionClosed: new EV(rmi, 'connectionClosed'),
    connectionOpened: new EV(rmi, 'connectionOpened')
};

rmi.start = function(url) {
    if (ws) return;
    if (!url) {
        let loc = window.location;
        url = (loc.protocol == 'https:' ? 'wss://' : 'ws://') + loc.host + '/ws';
    }
    ws = new WebSocket(url);
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

rmi.sendAsync = function(data, doNotBuffer) {
    if (ws.readyState != 1) {
        if (!doNotBuffer) waitingAsync.push(data);
    } else {
        ws.send(data);
    }
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

rmi.registerRSig = function(rsig) {
    rsig.id = ++rsigId;
    rsigHandlers[rsig.id] = rsig;
    sendRsigRegistration(rsig);
};

rmi.unregisterRSig = function(id) {
    let rsig = rsigHandlers[id];
    delete rsigHandlers[id];
    if (rsig) rmi.sendAsync(ber.S().s(rsig.service).s(rsig.name).i(false).i(rsig.id).box(2), true);
};

rmi.register = function(tagNo, func) { msgHandlers[tagNo] = func; };

rmi.setAliveTimeoutHandler = function(timoutMs, func) {
    lastAlive = performance.now();
    rmi.register(5, () => {
        lastAlive = performance.now();
        if (aliveTimeout) {
            aliveTimeout = false;
            func(false);
        }
    });
    setInterval(() => {
        if (performance.now() - lastAlive > timoutMs && !aliveTimeout) {
            aliveTimeout = true;
            func(true);
        }
    }, timoutMs / 2)
};

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

export default rmi;
