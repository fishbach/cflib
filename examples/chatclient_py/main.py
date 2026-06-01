# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

"""Python chat client for the cflib `chatserver` example.

Connects to the running chatserver over a WebSocket, subscribes to the
``newMessage`` remote signal (printing every broadcast message) and sends each
line typed on stdin as a chat message.  Run several clients (Python, the JS
browser client, ...) at once to see messages fan out.

Start the server first:

    ./bin/chatserver

then, from this directory:

    uv run python main.py
"""

import asyncio
import sys
from datetime import datetime, timezone
from pathlib import Path

# The generated service/DAO stubs live in the chatserver's `remote/` tree.  They
# import the `cflib` runtime (a real dependency) but are themselves just emitted
# source, so we put their package root on the import path.
_GENERATED = Path(__file__).resolve().parent.parent / "chatserver" / "remote" / "chatserver" / "py"
sys.path.insert(0, str(_GENERATED))

from cflib.net.rmi import RMI                         # noqa: E402
from chatserver.dao.message import Message           # noqa: E402  (generated)
from services.chatservice import ChatService         # noqa: E402  (generated)

URL = "ws://localhost:8000/ws"


def on_new_message(msg: Message):
    when = msg.timestamp.astimezone().strftime("%H:%M:%S") if msg.timestamp else "--:--:--"
    print(f"\r[{when}] {msg.text}\n> ", end="", flush=True)


async def read_stdin(prompt: str = "> ") -> str | None:
    """Read one line from stdin without blocking the event loop."""
    print(prompt, end="", flush=True)
    line = await asyncio.to_thread(sys.stdin.readline)
    if line == "":          # EOF (Ctrl-D)
        return None
    return line.rstrip("\n")


async def main():
    rmi = RMI(auto_reconnect=False)
    await rmi.connect(URL)
    chat = ChatService(rmi)

    # subscribe to broadcasts
    chat.newMessage.connect(on_new_message)
    await chat.newMessage.register()

    # demonstrate a request/reply round-trip
    print("server test() ->", await chat.test())
    print("server test2(41) ->", await chat.test2(41))   # (returnvalue, out-param)
    print("Type messages and press enter (Ctrl-D to quit).")

    try:
        while True:
            text = await read_stdin()
            if text is None:
                break
            if not text:
                continue
            await chat.sendMessage(Message(timestamp=datetime.now(timezone.utc), text=text))
    finally:
        await rmi.disconnect()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
