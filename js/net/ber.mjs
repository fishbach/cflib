/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

function fromUTF8(uint8Array)
{
    var se = '';
    var i = 0;
    var l = uint8Array.length;
    while (i < l) {
        var u8 = uint8Array[i++];
        /*jshint smarttabs:true */
        var u32 = u8 < 0x80 ?  u8 :
            u8 < 0xE0 ? (u8 & 0x1F) <<  6 | (uint8Array[i++] & 0x3F) :
            u8 < 0xF0 ? (u8 & 0x0F) << 12 | (uint8Array[i++] & 0x3F) <<  6 | (uint8Array[i++] & 0x3F) :
                        (u8 & 0x07) << 18 | (uint8Array[i++] & 0x3F) << 12 | (uint8Array[i++] & 0x3F) << 6 | (uint8Array[i++] & 0x3F)
        if (u32 < 0xFFFF) {
            se += String.fromCharCode(u32);
        } else {
            u32 -= 0x10000;
            var highSurrogate = 0xD800 | (u32 >> 10 & 0x3FF);
            var lowSurrogate = 0xDC00 | (u32 & 0x3FF);
            se += String.fromCharCode(highSurrogate, lowSurrogate);
        }
    }
    return se;
}

function toUTF8(str)
{
    var il = str.length;
    var rv = this.d;
    var oi = this.len;
    var isLatin = true;
    for (var i = 0 ; i < il ; ++i) {
        var c = str.charCodeAt(i);
        if (c < 0x80) rv[oi++] = c;
        else {
            if ((c & 0xFC00) == 0xD800) {
                var c2 = str.charCodeAt(i+1);
                if ((c2 & 0xFC00) == 0xDC00) {
                    c = ((c & 0x3FF) << 10 | (c2 & 0x3FF)) + 0x10000
                    ++i;
                }
                else continue;
            }

            // resize output array
            if (isLatin) {
                isLatin = false;
                /*jshint smarttabs:true */
                var ol =
                    c <   0x800 ? 1 :
                    c < 0x10000 ? 2 :
                                  3;
                for (var j = i + 1 ; j < il ; ++j) {
                    var cl = str.charCodeAt(j);
                    /*jshint smarttabs:true */
                    ol +=
                        cl <    0x80 ? 0 :
                        cl <   0x800 ? 1 :
                        cl < 0x10000 ? 2 :
                                       3;
                }
                this.len = oi;
                allocate.call(this, il - i + ol);
                rv = this.d;
            }

            if      (c <   0x800) { rv[oi++] = 0xC0 | c >>>  6; rv[oi++] = 0x80 | (c        & 0x3F); }
            else if (c < 0x10000) { rv[oi++] = 0xE0 | c >>> 12; rv[oi++] = 0x80 | (c >>>  6 & 0x3F); rv[oi++] = 0x80 | (c       & 0x3F); }
            else                  { rv[oi++] = 0xF0 | c >>> 18; rv[oi++] = 0x80 | (c >>> 12 & 0x3F); rv[oi++] = 0x80 | (c >>> 6 & 0x3F); rv[oi++] = 0x80 | (c & 0x3F); }
        }
    }
    this.len = oi;
}

// returns [tag, tagLen]
// returns -1 if not enough data available
function decodeBERTag(data, start, len)
{
    if (len < 1) return [-1, 0];
    var tag = data[start] & 0x1F;    // remove Class and P/C
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

// returns [valueLen, tag, tagLen, lengthSize]
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

var fa32 = new Float32Array(1);
var fa64 = new Float64Array(1);

function allocate(len)
{
    var need = this.len + len;
    if (this.d.length >= need) return;
    var newSize = this.d.length * 2;
    while (newSize < need) newSize *= 2;
    var old = this.d;
    this.d = new Uint8Array(newSize);
    this.d.set(new Uint8Array(old.buffer, 0, this.len));
}

function add(bytes)
{
    var l = bytes.length;
    allocate.call(this, l);
    this.d.set(bytes, this.len);
    this.len += l;
}

function prepareTagLen(estimatedLen)
{
    var tagPos = this.len;
    allocate.call(this, 2 + estimatedLen);
    this.len += 2;
    return tagPos;
}

function insertTagLen(tagPos)
{
    var sPos = tagPos + 2;
    var sLen = this.len - sPos;
    if (sLen === 0) {
        this.len -= 2;
        return this.n();
    }
    var tl = createTag(this.tagNo === 0 ? 0 : this.tagNo++).concat(encodeBERLength(sLen));
    if (tl.length > 2) {
        var n = tl.length - 2;
        allocate.call(this, n);
        this.len += n;
        this.d.set(new Uint8Array(this.d.buffer, sPos, sLen), sPos + n);
    }
    this.d.set(tl, tagPos);
    return this;
}

function Serializer(disableTagNumbering)
{
    this.tagNo = disableTagNumbering ? 0 : 1;
    this.d = new Uint8Array(4);
    this.len = 0;
}

Serializer.prototype = {

    data: function() { return new Uint8Array(this.d.buffer, 0, this.len); },

    box: function(tagNo) {
        return makeTLV(tagNo, true, this.data());
    },

    n: function() {
        if (this.tagNo === 0) add.call(this, [0xC0, 0x81, 0x00]);
        else                  this.tagNo++;
        return this;
    },

    z: function() {
        var tag = createTag(this.tagNo === 0 ? 0 : this.tagNo++);
        tag.push(0);
        add.call(this, tag);
        return this;
    },

    i: function(val) {
        if (!val) return this.n();

        var neg = val < 0;
        if (neg) val = -val - 1;
        var b = val & 0xFF;
        var bytes = [neg ? ~b : b];
        val = val > 0xFFFFFFFF ? Math.floor(val / 256) : val >>> 8;
        while (val > 0) {
            b = val & 0xFF;
            bytes.unshift(neg ? ~b : b);
            val = val > 0xFFFFFFFF ? Math.floor(val / 256) : val >>> 8;
        }
        if ((b & 0x80) !== 0) {
            if (neg) bytes.unshift(0xFF);
            else     bytes.unshift(0);
        }

        var tag = createTag(this.tagNo === 0 ? 0 : this.tagNo++);
        tag.push(bytes.length);
        add.call(this, tag.concat(bytes));
        return this;
    },

    f32: function(val) {
        if (!val) return this.n();

        var tag = createTag(this.tagNo === 0 ? 0 : this.tagNo++);
        tag.push(4);
        add.call(this, tag);
        fa32[0] = val;
        add.call(this, new Uint8Array(fa32.buffer));
        return this;
    },

    f64: function(val) {
        if (!val) return this.n();

        var tag = createTag(this.tagNo === 0 ? 0 : this.tagNo++);
        tag.push(8);
        add.call(this, tag);
        fa64[0] = val;
        add.call(this, new Uint8Array(fa64.buffer));
        return this;
    },

    s: function(str) {
        if (str === '') return this.z();
        if (!str)       return this.n();

        var tagPos = prepareTagLen.call(this, str.length);
        toUTF8.call(this, str);
        return insertTagLen.call(this, tagPos);
    },

    a: function(byteArray) {
        if (!byteArray            ) return this.n();
        if (byteArray.length === 0) return this.z();

        var tag = createTag(this.tagNo === 0 ? 0 : this.tagNo++);
        add.call(this, tag.concat(encodeBERLength(byteArray.length)));
        add.call(this, byteArray);
        return this;
    },

    o: function(obj, serFunc) {
        if (!obj) return this.n();

        var tagPos = prepareTagLen.call(this, 4);
        var oldTag = this.tagNo;
        this.tagNo = 1;

        if (serFunc) serFunc.call(obj, this);
        else         obj.__serialize(this);

        this.tagNo = oldTag;
        return insertTagLen.call(this, tagPos);
    },

    map: function(list, func) {
        if (!list || list.length === 0) return this.n();

        var tagPos = prepareTagLen.call(this, 4);
        var oldTag = this.tagNo;
        this.tagNo = 0;

        for (var i = 0, l = list.length ; i < l ; ++i) func(list[i], this);

        this.tagNo = oldTag;
        return insertTagLen.call(this, tagPos);
    },

    p: function(pair, func) {
        if (!pair) return this.n();

        var tagPos = prepareTagLen.call(this, 4);
        var oldTag = this.tagNo;
        this.tagNo = 1;

        func(pair, this);

        this.tagNo = oldTag;
        return insertTagLen.call(this, tagPos);
    }

};

function Deserializer(data, disableTagNumbering)
{
    this.data = data;
    this.start = 0;
    this.len = data ? data.length : 0;
    this.tagNo = disableTagNumbering ? 0 : 1;
}

Deserializer.prototype = {

    read: function() {
        for (;;) {
            // read tlv
            var tlv = decodeTLV(this.data, this.start, this.len);
            var valueLen = tlv[0];
            var tag = tlv[1];
            var tagLen = tlv[2];
            var lengthSize = tlv[3];
            var tlvLen = tagLen + lengthSize + valueLen;
            if (valueLen < 0 || this.len < tlvLen) break;

            // check tag and deserialize
            if (tag == this.tagNo) {
                var rv = valueLen === 0 && lengthSize == 2 ? null :
                    new Uint8Array(this.data.buffer, this.data.byteOffset + this.start + tagLen + lengthSize, valueLen);
                this.start += tlvLen;
                this.len   -= tlvLen;
                if (this.tagNo > 0) ++this.tagNo;
                return rv;
            } else if (tag > this.tagNo) {
                if (this.tagNo > 0) ++this.tagNo;
                return null;
            }

            // tag < this.tagNo
            this.start += tlvLen;
            this.len   -= tlvLen;
        }

        // some error occured
        this.len = 0;
        return null;
    },

    n: function() {
        ++this.tagNo;
    },

    i: function() {
        var raw = this.read();
        if (!raw) return 0;
        var s = 0;
        var e = raw.length;
        var rv = raw[s++];
        var neg = (rv & 0x80) !== 0;
        if (neg) rv = ~rv & 0xFF;
        while (s < e) rv = (rv * 256) + (neg ? ~raw[s++] & 0xFF : raw[s++]);
        return neg ? -rv - 1 : rv;
    },

    f32: function() {
        var raw = this.read();
        if (!raw || raw.length != 4) return 0;
        var a = new Uint8Array(4);
        a.set(raw);
        return (new Float32Array(a.buffer))[0];
    },

    f64: function() {
        var raw = this.read();
        if (!raw || raw.length != 8) return 0;
        var a = new Uint8Array(8);
        a.set(raw);
        return (new Float64Array(a.buffer))[0];
    },

    s: function() {
        var raw = this.read();
        if (!raw) return null;
        if (raw.length === 0) return '';
        return fromUTF8(raw);
    },

    a: function() {
        var raw = this.read();
        if (!raw) return null;
        return raw;
    },

    map: function(func) {
        var raw = this.read();
        if (!raw) return [];
        var D = new Deserializer(raw, true);
        var rv = [];
        while (D.len > 0) rv.push(func(D));
        return rv;
    }

};

// ========================================================================

var BER = function() {};
var ber = new BER();

ber.decodeTLV = decodeTLV;
ber.makeTLV = makeTLV;
ber.S = function(disableTagNumbering) { return new Serializer(disableTagNumbering); };
ber.D = function(data, disableTagNumbering) { return new Deserializer(data, disableTagNumbering); };

export default ber;
