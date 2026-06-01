# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

"""Value conversions used by generated (de)serialization code.

These mirror the inline conversions emitted by the JavaScript generator for the
``DateTime`` and ``tribool`` wire types.
"""

from datetime import datetime, timezone

__all__ = ["to_millis", "from_millis", "to_tribool", "from_tribool"]


def to_millis(dt):
    """datetime -> epoch milliseconds (0 for ``None``)."""
    if dt is None:
        return 0
    return int(dt.timestamp() * 1000)


def from_millis(ms):
    """epoch milliseconds -> aware UTC datetime (``None`` for 0/empty)."""
    if not ms:
        return None
    return datetime.fromtimestamp(ms / 1000, tz=timezone.utc)


def to_tribool(v):
    """tribool -> wire int: True->1, False->2, unknown(None)->0."""
    return 1 if v is True else 2 if v is False else 0


def from_tribool(v):
    """wire int -> tribool: 1->True, 2->False, else None."""
    return True if v == 1 else False if v == 2 else None
