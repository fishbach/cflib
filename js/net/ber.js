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

define(function() {

	// returns -1 if not enough data available
	function decodeBERTag(data, start, len)
	{
		if (len < 1) return [-1, 0];
		var tag = data[start] & 0x1F;	// remove Class and P/C
		var tagLen = 1;
		if (tag == 0x1F) {
			++tagLen;
			if (len < tagLen) return [-1, 0];
			tag <<= 8;
			tag |= data[++start];
			while (tag & 0x80) {
				++tagLen;
				if (len < tagLen) return [-1, 0];
				tag <<= 8;
				tag |= data[++start];
			}
		}
		return [tag, tagLen];
	}

	// returns -1 if not enough data available
	// returns -2 if length is undefined (one byte: 0x80)
	// returns -3 if too big length was found
	function decodeBERLength(data, start, len)
	{
		// Is some data available?
		if (len < 1) return [-1, 0];

		// If 8th bit is not set, length is in this byte.
		var b = data[start];
		if ((b & 0x80) === 0) return [b, 1];

		// 8th bit is set, so lower bits hold the size of the length
		var ls = b & 0x7F;
		if (ls > 8) return [-3, 0];
		if (len <= ls) return [-1, 0];

		// check for undefined length
		if (ls === 0) return [-2, 1];

		// check for too big length (signed qint64 overflow)
		b = data[++start];
		if (ls == 8 && ((b & 0x80) !== 0)) return [-3, 0];

		// calculate length
		var lengthSize = ls + 1;
		var retval = b;
		while (--ls > 0) retval = (retval << 8) | data[++start];
		return [retval, lengthSize];
	}

	// returns -1 if not enough data available
	// returns -2 if length is undefined (one byte: 0x80)
	// returns -3 if too big length was found
	function decodeTLV(data, start, len)
	{
		var rv = decodeBERTag(data, start, len);
		var tag = rv[0];
		var tagLen = rv[1];
		if (tag < 0) return [-1, 0, 0, 0];
		var rv2 = decodeBERLength(data, start + tagLen, len - tagLen);
		return [rv2[0], tag, tagLen, rv2[1]];
	}

	function createTag(tagNo, constructed)
	{
		if (tagNo < 0x1F) return [(constructed ? 0xE0 : 0xC0) | tagNo];

		var rv = [tagNo & 0x7F];
		tagNo >>>= 7;
		while (tagNo > 0) {
			rv.unshift((tagNo & 0x7F) | 0x80);
			tagNo >>>= 7;
		}
		rv.unshift(constructed ? 0xFF : 0xDF);
		return rv;
	}

	// If length < 0 the undefined length is written.
	function encodeBERLength(length)
	{
		// one byte encoding
		if (length < 0)    return [0x80];
		if (length < 0x80) return [length];

		var rv = [length & 0xFF];
		while (length > 0xFF) {
			length >>>= 8;
			rv.unshift(length & 0xFF);
		}
		rv.unshift(rv.length | 0x80);
		return rv;
	}

	function makeTLV(tagNo, constructed, uint8Array)
	{
		var tag = createTag(tagNo, constructed);
		if (uint8Array === null)                    { tag.push(0x81, 0); return new Uint8Array(tag); }
		if (!uint8Array || uint8Array.length === 0) { tag.push(0      ); return new Uint8Array(tag); }

		var valueLen = uint8Array.length;
		var len = encodeBERLength(valueLen);
		var valueStart = tag.length + len.length;
		var rv = new Uint8Array(valueStart + valueLen);
		rv.set(tag);
		rv.set(len, tag.length);
		rv.set(uint8Array, valueStart);
		return rv;
	}

	function Serializer(disableTagNumbering)
	{
		this.tagNo = 0;
		this.noInc = disableTagNumbering;
		this.data = new Uint8Array(0);
	}
	$.extend(Serializer.prototype, {

		add: function(d) {
			var od = this.data;
			this.data = new Uint8Array(od.length + d.length);
			this.data.set(od);
			this.data.set(d, od.length);
		},

		writeNull: function() { this.add([0xD0, 0x81, 0x00]); },

		i: function(val) {
			var tagNo = this.noInc ? this.tagNo : ++this.tagNo;
			if (!val) {
				if (tagNo === 0) this.writeNull();
				return this;
			}

			var neg = val < 0;
			if (neg) val = -val - 1;
			var b = val & 0xFF;
			var bytes = [neg ? ~b : b];
			val = val > 0xFFFFFFFF ? Math.floor(val / 16) : val >>> 8;
			while (val > 0) {
				b = val & 0xFF;
				bytes.unshift(neg ? ~b : b);
				val = val > 0xFFFFFFFF ? Math.floor(val / 16) : val >>> 8;
			}
			if ((b & 0x80) !== 0) {
				if (neg) bytes.unshift(0xFF);
				else     bytes.unshift(0);
			}

			var tag = createTag(tagNo);
			tag.push(bytes.length);
			this.add(tag.concat(bytes));
			return this;
		},

		f64: function(val) {
			var tagNo = this.noInc ? this.tagNo : ++this.tagNo;
			if (!val) {
				if (tagNo === 0) this.writeNull();
				return this;
			}

			var tag = createTag(tagNo);
			tag.push(8);
			this.add(tag);
			var fa = new Float64Array(1);
			fa[0] = val;
			this.add(new Uint8Array(fa.buffer));
			return this;
		}

	});

	// ========================================================================

	var BER = function() {};
	var ber = new BER();

	ber.decodeTLV = decodeTLV;
	ber.makeTLV = makeTLV;
	ber.S = function() { return new Serializer(); };

	return ber;
});
