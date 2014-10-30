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

define([
	'cflib/util/ajax',
	'cflib/util/api',
	'cflib/ext/ZeroClipboard'
], function(ajax, api, ZeroClipboard) {

	ZeroClipboard.config( { swfPath: '/swf/cflib/ZeroClipboard.swf' } );

	var timeDiff = 0;

	function logToServer(category, str)
	{
		var file = 'javascript';
		var line = 0;
		try { util.__undefined(); } catch (e) { try {
			var parts = /.+\/[^\/]+\/[^\/]+.js:\d+.*\n.+\/[^\/]+\/[^\/]+.js:\d+.*\n.+\/([^\/]+\/[^\/]+.js):(\d+)/.exec(e.stack);
			file = parts[1];
			line = +parts[2];
		} catch (e2) {} }
		api.doAsyncRMI('logservice', ['void log(String,int32,uint16,String)', file, line, category, str]);
	}

	// ========================================================================

	var Util = function() {};
	var util = new Util();

	util.simplifyStr = function(str) {
		return (str
			.replace(/^\s+/, '')
			.replace(/\s+$/, '')
			.replace(/\s+/g, ' '));
	};

	util.getInputStr = function(el) {
		return util.simplifyStr($(el).val()).replace(/[^\w@\._\- ]+/g, '');
	};

	util.dateFromUTC = function(date) {
		return new Date(date.getUTCFullYear(), date.getUTCMonth(), date.getUTCDate());
	};

	util.shake = function($el) {
		var oldPos  = $el.css('position');
		var oldLeft = $el.css('left');
		$el.css('position', 'relative');
		var elCount = $el.length;
		var finished = function() {
			if (--elCount) return;
			$el.css('position', oldPos).css('left', oldLeft);
		};
		var i = 3;
		while (i--) {
			$el
				.animate({ left: -5 }, 40)
				.animate({ left:  5 }, 80)
				.animate({ left:  0 }, 40, !i ? finished : undefined);
		}
	};

	util.debounce = function(func, ms) {
		var timer = null;
		return function() {
			var scope = this;
			var param = arguments;
			if (timer) clearTimeout(timer);
			timer = setTimeout(function() {
				timer = null;
				func.apply(scope, param);
			}, ms ? ms : 50);
		};
	};

	util.defer = function(func) {
		return function() {
			var scope = this;
			var param = arguments;
			setTimeout(function() { func.apply(scope, param); }, 0);
		};
	};

	util.removeFromArray = function(array, el) {
		// unfortunately IE 8 does not have 'indexOf' on Arrays
		var i = array.length;
		while (i--) if (array[i] === el) {
			array.splice(i, 1);
			return true;
		}
		return false;
	};

	util.validInputChars = function(str) {
		str = str.replace(/^\s+|\s+$/g, '').replace(/\s+/, ' ');
		if (/[{}\[\]<>;"\\]/.test(str)) return null;
		return str;
	};

	util.formatInt = function(i, w) {
		var r = '' + i;
		while (r.length < w) r = '0' + r;
		return r;
	};

	util.formatWeekDay = function(date) {
		switch (date.getUTCDay()) {
			case 0: return 'Sun';
			case 1: return 'Mon';
			case 2: return 'Tue';
			case 3: return 'Wed';
			case 4: return 'Thu';
			case 5: return 'Fri';
			case 6: return 'Sat';
		}
	};

	util.isWeekend = function(date) {
		switch (date.getUTCDay()) {
			case 0:
			case 6:  return true;
			default: return false;
		}
	};

	util.formatDateTime = function(date, withSecs, withWeekDay, noNbsp) {
		return (
			(withWeekDay ? util.formatWeekDay(date) + (noNbsp ? ' ' : '&nbsp;') : '') +
			util.formatInt(date.getDate(), 2) + '.' +
			util.formatInt(date.getMonth() + 1, 2) + '.' +
			util.formatInt(date.getFullYear(), 4) + (noNbsp ? ' ' : '&nbsp;') +
			util.formatInt(date.getHours(), 2) + ':' +
			util.formatInt(date.getMinutes(), 2) +
			(withSecs ? ':' + util.formatInt(date.getSeconds(), 2) : '')
		);
	};

	util.formatDayMonth = function(date) {
		return (
			util.formatInt(date.getUTCDate(), 2) + '.' +
			util.formatInt(date.getUTCMonth() + 1, 2) + '.'
		);
	};

	util.endsWith = function(str, part) {
		return (str.lastIndexOf(part) == (str.length - part.length));
	};

	// time = 150
	util.scrollTop = function(time, finishCb, scope) {
		if (time === undefined) {
			window.scrollTo(0, 0);
		} else {
			var cb = scope ? function() { finishCb.call(scope); } : finishCb;
			$('html, body').animate({ scrollTop : 0 }, time, cb);
		}
	};

	util.dateIsValid = function(date) {
		return (
			Object.prototype.toString.call(date) === "[object Date]" &&
			!isNaN(date.getTime())
		);
	};

	util.setRefTime = function(date) {
		timeDiff = date.getTime() - new Date().getTime();
	};

	util.now = function(doFormat) {
		if (doFormat) return util.formatDateTime(util.now(), false, true);
		return new Date(new Date().getTime() + timeDiff);
	};

	util.validEmail = function(str) {
		str = util.validInputChars(str);
		if (str === null || !/^[\w.\-_]+@\w[\w.\-]+\.\w+$/.test(str)) return null;
		return str;
	};

	util.logInfo     = function(str) { logToServer(4, str); };
	util.logWarn     = function(str) { logToServer(5, str); };
	util.logCritical = function(str) { logToServer(6, str); };

	util.initClipboardButton = function(button, dataFunc) {
		new ZeroClipboard(button).on('copy', function(e) {
			e.clipboardData.setData('text/plain', dataFunc());
		});
	};

	util.getArrayBuffer = function(url, callback, scope) {
		ajax.request('GET', url, function(status, buf, xhr) { callback.call(scope, buf); }, null, null, null, 'arraybuffer');
	};

	util.toHex = function(n, len) {
		var rv = n.toString(16);
		if (len) while (rv.length < len) rv = '0' + rv;
		return rv;
	};

	return util;
});
