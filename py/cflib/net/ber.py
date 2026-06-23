# Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
#
# This file is part of cflib.
#
# Licensed under the MIT License.

"""Hand-written BER (ASN.1) serialization for cflib.

This is a direct port of ``js/net/ber.js`` and produces byte-identical output to
the JavaScript and C++ implementations.  It deliberately avoids any third party
encoding library; only Python built-ins (``bytearray``, ``struct``, the UTF-8
codec) are used.

Wire model: every value is a TLV (tag / length / value).  Inside a structure the
serializer auto-numbers tags starting at 1 (``tag_no``); ``tag_no == 0`` disables
numbering so every element shares tag 0 (used for list/map elements).
"""

import struct

__all__ = ["Serializer", "Deserializer", "S", "D", "decode_tlv", "make_tlv"]


# ---------------------------------------------------------------------------
# low level tag / length helpers
# ---------------------------------------------------------------------------

def create_tag(tag_no, constructed=False):
    """Encode a tag number (private class) as bytes."""
    if tag_no < 0x1F:
        return bytes([(0xE0 if constructed else 0xC0) | tag_no])

    rv = [tag_no & 0x7F]
    tag_no >>= 7
    while tag_no > 0:
        rv.insert(0, (tag_no & 0x7F) | 0x80)
        tag_no >>= 7
    rv.insert(0, 0xFF if constructed else 0xDF)
    return bytes(rv)


def encode_ber_length(length):
    """Encode a definite length.  A negative length yields the indefinite form."""
    if length < 0:
        return b"\x80"
    if length < 0x80:
        return bytes([length])

    rv = [length & 0xFF]
    while length > 0xFF:
        length >>= 8
        rv.insert(0, length & 0xFF)
    rv.insert(0, len(rv) | 0x80)
    return bytes(rv)


def make_tlv(tag_no, constructed=False, value=None):
    """Build a complete TLV frame.

    ``value`` of ``None`` encodes an explicit null (length ``0x81 00``); an empty
    bytes object encodes an empty value (length ``00``).
    """
    tag = create_tag(tag_no, constructed)
    if value is None:
        return tag + b"\x81\x00"
    if len(value) == 0:
        return tag + b"\x00"
    return tag + encode_ber_length(len(value)) + bytes(value)


def decode_ber_tag(data, start, length):
    """Return ``(tag, tag_len)``; ``tag`` is -1 when not enough data is available."""
    if length < 1:
        return -1, 0
    tag = data[start] & 0x1F  # strip class and primitive/constructed bits
    tag_len = 1
    if tag == 0x1F:
        idx = start
        tag_len += 1
        if length < tag_len:
            return -1, 0
        idx += 1
        tag = (tag << 8) | data[idx]
        while tag & 0x80:
            tag_len += 1
            if length < tag_len:
                return -1, 0
            idx += 1
            tag = (tag << 8) | data[idx]
    return tag, tag_len


def decode_ber_length(data, start, length):
    """Return ``(value_len, length_size)``.

    ``value_len`` is -1 (not enough data), -2 (indefinite length) or -3 (too big).
    """
    if length < 1:
        return -1, 0

    b = data[start]
    if (b & 0x80) == 0:
        return b, 1

    ls = b & 0x7F
    if ls > 8:
        return -3, 0
    if length <= ls:
        return -1, 0
    if ls == 0:
        return -2, 1

    idx = start + 1
    b = data[idx]
    if ls == 8 and (b & 0x80) != 0:
        return -3, 0

    length_size = ls + 1
    retval = b
    while ls > 1:
        ls -= 1
        idx += 1
        retval = (retval << 8) | data[idx]
    return retval, length_size


def decode_tlv(data, start, length):
    """Return ``(value_len, tag, tag_len, length_size)`` for the TLV at ``start``."""
    tag, tag_len = decode_ber_tag(data, start, length)
    if tag < 0:
        return -1, 0, 0, 0
    value_len, length_size = decode_ber_length(data, start + tag_len, length - tag_len)
    return value_len, tag, tag_len, length_size


def _int_to_minimal_bytes(val):
    """Minimal-length big-endian two's complement encoding of a non-zero int."""
    length = 1
    while True:
        try:
            return val.to_bytes(length, "big", signed=True)
        except OverflowError:
            length += 1


# ---------------------------------------------------------------------------
# Serializer
# ---------------------------------------------------------------------------

class Serializer:
    """Builds a BER byte stream.  Chainable: every method returns ``self``."""

    __slots__ = ("tag_no", "_buf")

    def __init__(self, disable_tag_numbering=False):
        self.tag_no = 0 if disable_tag_numbering else 1
        self._buf = bytearray()

    def _tag(self):
        """Return the current tag number, advancing the counter when numbering is on."""
        if self.tag_no == 0:
            return 0
        t = self.tag_no
        self.tag_no += 1
        return t

    def data(self):
        """The serialized payload built so far."""
        return bytes(self._buf)

    def box(self, tag_no):
        """Wrap the whole payload in a constructed TLV with the given tag."""
        return make_tlv(tag_no, True, self._buf)

    def n(self):
        """Null / absent value (skips a tag when numbering, else writes explicit null)."""
        if self.tag_no == 0:
            self._buf += b"\xC0\x81\x00"
        else:
            self.tag_no += 1
        return self

    def z(self):
        """Empty (zero length) value."""
        self._buf += create_tag(self._tag())
        self._buf += b"\x00"
        return self

    def i(self, val):
        if not val:
            return self.n()
        payload = _int_to_minimal_bytes(int(val))
        self._buf += create_tag(self._tag())
        self._buf += bytes([len(payload)])
        self._buf += payload
        return self

    def f32(self, val):
        if not val:
            return self.n()
        self._buf += create_tag(self._tag())
        self._buf += b"\x04"
        self._buf += struct.pack("<f", val)
        return self

    def f64(self, val):
        if not val:
            return self.n()
        self._buf += create_tag(self._tag())
        self._buf += b"\x08"
        self._buf += struct.pack("<d", val)
        return self

    def s(self, val):
        if val is None:
            return self.n()
        if val == "":
            return self.z()
        data = val.encode("utf-8")
        self._buf += create_tag(self._tag())
        self._buf += encode_ber_length(len(data))
        self._buf += data
        return self

    def a(self, val):
        if val is None:
            return self.n()
        if len(val) == 0:
            return self.z()
        self._buf += create_tag(self._tag())
        self._buf += encode_ber_length(len(val))
        self._buf += bytes(val)
        return self

    def _wrap(self, payload):
        """Write ``payload`` as a primitive-class TLV using the current tag."""
        if len(payload) == 0:
            return self.n()
        self._buf += create_tag(self._tag())
        self._buf += encode_ber_length(len(payload))
        self._buf += payload
        return self

    def o(self, obj, ser_func=None):
        """Serialize a nested object (tag numbering restarts at 1 inside)."""
        if obj is None:
            return self.n()
        child = Serializer()
        if ser_func is not None:
            ser_func(obj, child)
        else:
            obj._serialize(child)
        return self._wrap(child._buf)

    def map(self, items, func):
        """Serialize a sequence; elements share tag 0 (numbering disabled)."""
        if not items:
            return self.n()
        child = Serializer(disable_tag_numbering=True)
        for e in items:
            func(e, child)
        return self._wrap(child._buf)

    def p(self, pair, func):
        """Serialize a pair (tag numbering restarts at 1 inside)."""
        if pair is None:
            return self.n()
        child = Serializer()
        func(pair, child)
        return self._wrap(child._buf)


# ---------------------------------------------------------------------------
# Deserializer
# ---------------------------------------------------------------------------

class Deserializer:
    """Reads a BER byte stream produced by :class:`Serializer`."""

    __slots__ = ("data", "start", "len", "tag_no")

    def __init__(self, data, disable_tag_numbering=False):
        self.data = data if data is not None else b""
        self.start = 0
        self.len = len(self.data)
        self.tag_no = 0 if disable_tag_numbering else 1

    def read(self):
        """Read the value bytes for the next expected tag (``None`` if absent)."""
        while True:
            value_len, tag, tag_len, length_size = decode_tlv(self.data, self.start, self.len)
            tlv_len = tag_len + length_size + value_len
            if value_len < 0 or self.len < tlv_len:
                break

            if tag == self.tag_no:
                if value_len == 0 and length_size == 2:
                    rv = None
                else:
                    vstart = self.start + tag_len + length_size
                    rv = self.data[vstart:vstart + value_len]
                self.start += tlv_len
                self.len -= tlv_len
                if self.tag_no > 0:
                    self.tag_no += 1
                return rv

            if tag > self.tag_no:
                if self.tag_no > 0:
                    self.tag_no += 1
                return None

            # tag < self.tag_no -> skip this element and keep looking
            self.start += tlv_len
            self.len -= tlv_len

        self.len = 0
        return None

    def n(self):
        """Skip one tag position."""
        self.tag_no += 1

    def i(self):
        raw = self.read()
        if not raw:
            return 0
        return int.from_bytes(raw, "big", signed=True)

    def f32(self):
        raw = self.read()
        if not raw or len(raw) != 4:
            return 0.0
        return struct.unpack("<f", raw)[0]

    def f64(self):
        raw = self.read()
        if not raw or len(raw) != 8:
            return 0.0
        return struct.unpack("<d", raw)[0]

    def s(self):
        raw = self.read()
        if raw is None:
            return None
        if len(raw) == 0:
            return ""
        return raw.decode("utf-8")

    def a(self):
        raw = self.read()
        if raw is None:
            return None
        return bytes(raw)

    def map(self, func):
        """Deserialize a sequence, calling ``func(D)`` for each element."""
        raw = self.read()
        if not raw:
            return []
        d = Deserializer(raw, disable_tag_numbering=True)
        rv = []
        while d.len > 0:
            rv.append(func(d))
        return rv


# ---------------------------------------------------------------------------
# module level convenience (mirrors the `ber` default export in ber.js)
# ---------------------------------------------------------------------------

def S(disable_tag_numbering=False):
    return Serializer(disable_tag_numbering)


def D(data, disable_tag_numbering=False):
    return Deserializer(data, disable_tag_numbering)
