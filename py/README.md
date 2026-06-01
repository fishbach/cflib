# cflib — Python client runtime

Python3 client runtime for [cflib](../README.md), mirroring the JavaScript runtime
in `js/net/`. It lets a Python program call a cflib C++ server over a WebSocket
using the same BER (ASN.1) binary protocol used by the C++ and JavaScript clients.

- `cflib.net.ber` — hand-written BER (de)serialization (no third-party encoding lib).
- `cflib.net.rmi` — asyncio RMI client (one request in flight, RSig dispatch).
- `cflib.net.rsig` — remote-signal subscription.

The only third-party dependency is [`websockets`](https://pypi.org/project/websockets/)
for the transport. Managed with [uv](https://docs.astral.sh/uv/).

Service and DAO classes are **generated** from the server's service definitions
(see `cflib/serialize/generate/python.cpp`), the same way the `.mjs` stubs are
generated for the browser. See `examples/chatclient_py/` for a complete client.
