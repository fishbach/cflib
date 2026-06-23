# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

"""Asyncio RMI client — WebSocket transport for cflib's BER protocol.

Port of ``js/net/rmi.js``.  One WebSocket carries several logical channels,
distinguished by the BER tag of each binary frame:

* tag 1 — client-id handshake (sent on connect, echoed/assigned by the server)
* tag 2 — request/reply.  There are **no request ids**: replies are matched to
  requests purely by order, so at most one request may be in flight at a time.
  This is enforced here with an :class:`asyncio.Lock`.
* tag 3 — remote-signal (RSig) fire
* tag 4 — server ping (echoed back verbatim)
* tag 5 — keep-alive ping

The only third-party dependency is ``websockets`` (the transport); BER itself is
implemented by hand in :mod:`cflib.net.ber`.
"""

import asyncio

from websockets.asyncio.client import connect as ws_connect

from . import ber

__all__ = ["RMI"]

_RECONNECT_DELAY = 5.0


class RMI:
    def __init__(self, *, auto_reconnect=True):
        self._url = None
        self._headers = None
        self._auto_reconnect = auto_reconnect

        self._ws = None
        self._recv_task = None
        self._closing = False
        self._connected = asyncio.Event()

        self.client_id = None             # bytes assigned by the server
        self.on_identity_reset = None     # optional callback(client_id)
        self.on_connected = None          # optional callback()
        self.on_disconnected = None       # optional callback()

        # one request in flight at a time (the protocol has no request ids)
        self._req_lock = asyncio.Lock()
        self._pending_reply = None        # asyncio.Future for the active request

        self._rsig_handlers = {}          # id -> RSig
        self._next_rsig_id = 0

    # -- connection -------------------------------------------------------

    async def connect(self, url, *, client_id=None, headers=None):
        """Open the connection and complete the handshake send."""
        self._url = url
        self._headers = headers
        self._closing = False
        if client_id is not None:
            self.client_id = client_id
        await self._open()

    async def _open(self):
        self._ws = await ws_connect(self._url, additional_headers=self._headers, max_size=None)

        # handshake: tag 1, optionally carrying a previously assigned client id
        if self.client_id:
            await self._ws.send(ber.make_tlv(1, False, self.client_id))
        else:
            await self._ws.send(ber.make_tlv(1))

        self._connected.set()
        self._recv_task = asyncio.create_task(self._recv_loop())

        # re-subscribe all signals (relevant after a reconnect)
        for rsig in list(self._rsig_handlers.values()):
            await self._send_rsig_registration(rsig, True)

        if self.on_connected:
            self.on_connected()

    async def disconnect(self):
        """Close the connection and stop reconnecting."""
        self._closing = True
        self._connected.clear()
        ws = self._ws
        self._ws = None
        if ws is not None:
            await ws.close()
        if self._recv_task is not None:
            self._recv_task.cancel()

    @property
    def is_connected(self):
        return self._connected.is_set()

    async def wait_connected(self):
        await self._connected.wait()

    # -- receive / dispatch ----------------------------------------------

    async def _recv_loop(self):
        try:
            async for msg in self._ws:
                if isinstance(msg, (bytes, bytearray)):
                    self._dispatch(bytes(msg))
        except asyncio.CancelledError:
            raise
        except Exception:
            pass
        finally:
            self._handle_close()

    def _dispatch(self, data):
        value_len, tag, tag_len, length_size = ber.decode_tlv(data, 0, len(data))
        if value_len < 0:
            return
        vstart = tag_len + length_size
        value = data[vstart:vstart + value_len]

        if tag == 1:
            self.client_id = bytes(value)
            if self.on_identity_reset:
                self.on_identity_reset(self.client_id)
        elif tag == 2:
            fut = self._pending_reply
            self._pending_reply = None
            if fut is not None and not fut.done():
                fut.set_result(bytes(value))
        elif tag == 3:
            d = ber.D(value)
            rsig_id = d.i()
            params = d.a()
            rsig = self._rsig_handlers.get(rsig_id)
            if rsig is not None:
                rsig._fire(params)
        elif tag == 4:
            # server ping -> echo the whole frame back
            asyncio.create_task(self._raw_send(data))
        elif tag == 5:
            pass  # keep-alive

    def _handle_close(self):
        self._connected.clear()
        self._ws = None

        fut = self._pending_reply
        self._pending_reply = None
        if fut is not None and not fut.done():
            fut.set_exception(ConnectionError("RMI connection closed"))

        if self.on_disconnected:
            self.on_disconnected()

        if self._auto_reconnect and not self._closing:
            asyncio.create_task(self._reconnect())

    async def _reconnect(self):
        await asyncio.sleep(_RECONNECT_DELAY)
        if self._closing:
            return
        try:
            await self._open()
        except Exception:
            if not self._closing:
                asyncio.create_task(self._reconnect())

    # -- send -------------------------------------------------------------

    async def _raw_send(self, data):
        ws = self._ws
        if ws is not None:
            await ws.send(data)

    async def send_async(self, data):
        """Fire-and-forget send (waits until connected)."""
        await self._connected.wait()
        await self._ws.send(data)

    async def send_request(self, data):
        """Send a request and await its reply payload (bytes).

        Serialized via a lock so only one request is outstanding, matching the
        order-based reply correlation of the protocol.
        """
        async with self._req_lock:
            await self._connected.wait()
            loop = asyncio.get_running_loop()
            fut = loop.create_future()
            self._pending_reply = fut
            await self._ws.send(data)
            return await fut

    # -- RSig registry ----------------------------------------------------

    def _next_id(self):
        self._next_rsig_id += 1
        return self._next_rsig_id

    async def register_rsig(self, rsig):
        rsig.id = self._next_id()
        self._rsig_handlers[rsig.id] = rsig
        await self._send_rsig_registration(rsig, True)

    async def unregister_rsig(self, rsig):
        self._rsig_handlers.pop(rsig.id, None)
        await self._send_rsig_registration(rsig, False)

    async def _send_rsig_registration(self, rsig, subscribe):
        data = (ber.S()
                .s(rsig.service)
                .s(rsig.name)
                .i(1 if subscribe else 0)
                .i(rsig.id)
                .box(2))
        await self.send_async(data)
