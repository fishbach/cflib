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
		if ((b & 0x80) == 0) return [b, 1];

		// 8th bit is set, so lower bits hold the size of the length
		var ls = b & 0x7F;
		if (ls > 8) return [-3, 0];
		if (len <= ls) return [-1, 0];

		// check for undefined length
		if (ls == 0) return [-2, 1];

		// check for too big length (signed qint64 overflow)
		b = data[++start];
		if (ls == 8 && ((b & 0x80) != 0)) return [-3, 0];

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

	// ========================================================================

	var BER = function() {};
	var ber = new BER();

	ber.decodeTLV = decodeTLV;


	return ber;
});
