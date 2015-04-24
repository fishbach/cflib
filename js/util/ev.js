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

	var logOn = false;
	var uniqueId = 0;
	var indent = 0;

	function ind()
	{
		var rv = '';
		var i = indent++;
		while (i--) rv += '. ';
		return rv;
	}

	function unbindFromAll(obj)
	{
		var evData = obj.__evData;
		if (!evData) return;
		for (var id in evData) {
			var data = evData[id];
			var listener = data.event.listener;
			var ids = data.ids;
			var i = ids.length;
			while (i--) delete listener[ids[i]];
			delete evData[id];
		}
	}

	function doFire()
	{
		var ls = this.listener;
		var keys = []; for (var k in ls) keys.push(k);
		var len = keys.length;
		if (!this.enabled || len === 0) return;

		var args = Array.prototype.slice.call(arguments);	// make real array from arguments
		args.push(this.source);

		var i, l;
		if (logOn) {
			console.log(ind() + 'fire ', this.name, '(', this.id, ') of ', this.source);
			for (i = 0 ; i < len ; ++i) {
				l = ls[keys[i]];
				if (l) {
					console.log(ind() + 'deliver to ', l[1], ' with arguments ', args);
					l[0].apply(l[1], args);
					--indent;
				}
			}
			--indent;
		} else {
			for (i = 0 ; i < len ; ++i) {
				l = ls[keys[i]];
				if (l) l[0].apply(l[1], args);
			}
		}
	}

	// ========================================================================

	var EV = function(source, name, throttle) {
		this.id = ++uniqueId;
		this.listenerId = 0;
		this.source = source;
		this.name = name;
		this.throttle = throttle === true ? 50 : throttle;
		this.throttleLast = new Date().getTime();
		this.throttleParams = null;
		this.enabled = true;
		this.listener = {};
	};

	EV.unbindAll = function(obj) {
		// unbind event forwards
		if (obj.ev) for (var name in obj.ev) unbindFromAll(obj.ev[name]);
		unbindFromAll(obj);
	};

	$.extend(EV.prototype, {

		// While an event is in process, it may happen that the listeners of that event change.
		// In this case newly added or removed listeners will not be called.
		fire: function() {
			if (!this.throttle) return doFire.apply(this, arguments);
			if (this.throttleParams) {
				this.throttleParams = arguments;
				return;
			}
			var now = new Date().getTime();
			if (now - this.throttleLast >= this.throttle) {
				this.throttleLast = now;
				doFire.apply(this, arguments);
				return;
			}
			var ev = this;
			this.throttleParams = arguments;
			setTimeout(function() {
				var p = ev.throttleParams;
				ev.throttleLast = new Date().getTime();
				ev.throttleParams = null;
				doFire.apply(ev, p);
			}, this.throttle - now + this.throttleLast);
		},

		forward: function(ev) {
			this.bind(ev.fire, ev);
		},

		bind: function(func, scope) {
			var id = ++(this.listenerId);
			if (scope) {
				var data = scope.__evData;
				if (!data) scope.__evData = data = {};
				var myData = data[this.id];
				if (!myData) data[this.id] = myData = { event: this, ids: [] };
				myData.ids.push(id);
			}
			this.listener[id] = [func, scope];
		},

		unbindAll: function(scope) {
			var evData = scope.__evData;
			var listener = this.listener;
			var ids = evData[this.id].ids;
			var i = ids.length;
			while (i--) delete listener[ids[i]];
			delete evData[this.id];
		},

		setEnabled: function(enabled) {
			this.enabled = enabled;
		}

	});

	return EV;
});
