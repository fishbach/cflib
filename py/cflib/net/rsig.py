# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

"""Remote signals (RSig) — server-pushed events a client can subscribe to.

Port of ``js/net/rsig.mjs``.  A generated service exposes one :class:`RSig` per
``cfsignals`` member.  Subscribe with :meth:`connect` and activate the server-side
subscription with ``await rsig.register()``.
"""

import asyncio

from . import ber

__all__ = ["RSig"]


class RSig:
    def __init__(self, rmi, service, name, deserialize):
        self._rmi = rmi
        self.service = service
        self.name = name
        # deserialize(Deserializer) -> tuple of fired arguments
        self._deserialize = deserialize
        self.id = 0
        self._listeners = []

    def connect(self, callback):
        """Register a listener.  ``callback`` may be a coroutine function."""
        self._listeners.append(callback)
        return self

    def disconnect(self, callback):
        try:
            self._listeners.remove(callback)
        except ValueError:
            pass
        return self

    async def register(self):
        """Subscribe to this signal on the server."""
        await self._rmi.register_rsig(self)
        return self

    async def unregister(self):
        """Cancel the server-side subscription."""
        await self._rmi.unregister_rsig(self)
        return self

    def _fire(self, params_data):
        """Decode the payload and notify all listeners (called by the RMI loop)."""
        args = self._deserialize(ber.D(params_data))
        for cb in list(self._listeners):
            res = cb(*args)
            if asyncio.iscoroutine(res):
                asyncio.create_task(res)
