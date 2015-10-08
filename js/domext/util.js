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

require([
	'cflib/dom'
], function($) {

	var animEls   = [];
	var animTimer = null;
	var wheelYMin = Infinity;
	var wheelYSeen = [];
	var unitRE = /^\s*([-\d.]+)\s*(\D*)\s*$/;

	function splitUnit(props)
	{
		var rv = {};
		for (var p in props) {
			var parts = unitRE.exec(props[p]);
			if (parts) rv[p] = [+parts[1], parts[2]];
		}
		return rv;
	}

	function setAnimProps($el, props)
	{
		for (var p in props) {
			var v = props[p];
			if (p == 'scrollTop') {
				window.scrollTo($.scrollPos()[0], v[0]);
			} else {
				$el.css(p, v[0] + v[1]);
			}
		}
	}

	function getAnimProps($el)
	{
		var rv = {};
		var props = $el.anim.prop;
		for (var p in props) {
			var u = props[p][1];
			var is;
			if (p == 'scrollTop') is = $.scrollPos()[1];
			else {
				var comp = unitRE.exec($el.computed(p));
				is = comp ? +comp[1] : 0;
				if (u == '%' && (!comp || comp[2] != '%')) {
					var rel =
						p == 'height' ||
						p == 'top'         || p == 'bottom' ||
						p == 'margin-top'  || p == 'margin-bottom' ||
						p == 'padding-top' || p == 'padding-bottom' ?
						$el.parent().height() : $el.parent().width();
					is = rel ? 100 * is / rel : 0;
				}
			}
			rv[p] = [is, u];
		}
		return rv;
	}

	function animStep()
	{
		var now = new Date().getTime();
		var remain = [];
		$.each(animEls, function(el) {
			var anim = el.anim;
			while (true) {
				var td = now - anim.start;
				if (td >= anim.duration) {
					var next = anim.queue.shift();
					if (next) {
						anim.start     += anim.duration;
						anim.startProp = anim.prop;
						anim.prop      = next[0];
						anim.duration  = next[1];
						anim.finish    = next[2];
						anim.easing    = next[3];
						continue;
					}

					setAnimProps(el, anim.prop);
					delete el.anim;
					if (anim.finish) anim.finish.call(el);
				} else {
					remain.push(el);
					var nProp = {};
					var sProp = anim.startProp;
					var eProp = anim.prop;
					var p = anim.easing(td / anim.duration);
					for (var prop in sProp) {
						var sp = sProp[prop];
						nProp[prop] = [eProp[prop][0] * p + sp[0] * (1 - p), sp[1]];
					}
					setAnimProps(el, nProp);
				}

				break;
			}
		});

		animEls = remain;
		if (animEls.length === 0) {
			clearInterval(animTimer);
			animTimer = null;
		}
	}

	// ========================================================================

	$.easing = {
		linear : function(p) { return p; },
		swing  : function(p) { return (1 - Math.cos(p * Math.PI)) / 2; }
	};

	$.fn.animate = function(prop, ms, finishFunc, easing) {
		if (!ms) ms = 400;
		else if (!easing) easing = $.easing.swing;

		if (this.anim) {
			this.anim.queue.push([splitUnit(prop), ms, finishFunc, easing]);
			return this;
		}

		this.anim = {};
		this.anim.queue = [];
		this.anim.prop = splitUnit(prop);
		this.anim.duration = ms;
		this.anim.finish = finishFunc;
		this.anim.easing = easing;
		this.anim.start = new Date().getTime();
		this.anim.startProp = getAnimProps(this);
		animEls.push(this);
		if (!animTimer) animTimer = setInterval(animStep, 13);
		return this;
	};

	$.fn.stop = function(setFinishState) {
		if (!this.anim) return this;
		if (setFinishState) {
			var q = this.anim.queue;
			setAnimProps(this, q.length > 0 ? q[q.length - 1][0]: this.anim.prop);
		}
		delete this.anim;
		var i = animEls.length;
		while (i--) {
			if (animEls[i] == this) {
				animEls.splice(i, 1);
				if (animEls.length === 0) {
					clearInterval(animTimer);
					animTimer = null;
				}
				return this;
			}
		}
	};

	$.fn.shake = function() {
		var oldPos  = this.css('position');
		var oldLeft = this.css('left');
		this.css('position', 'relative');
		var elCount = this.count();
		var finished = function() {
			if (--elCount) return;
			this.css('position', oldPos).css('left', oldLeft);
		};
		var i = 3;
		while (i--) {
			this
				.animate({ left: '-5px' }, 40)
				.animate({ left:  '5px' }, 80)
				.animate({ left:  '0px' }, 40, !i ? finished : undefined);
		}
		return this;
	};

	// time = 150
	$.scrollTop = function(time, finishCb, scope) {
		if (time === undefined) {
			window.scrollTo(0, 0);
		} else {
			var cb = scope ? function() { finishCb.call(scope); } : finishCb;
			$.all('html, body').animate({ scrollTop : 0 }, time, cb);
		}
	};

	$.wheelAdjustY = function(e) {
		var dy = e.deltaY;
		var abs = Math.abs(dy);
		if (abs < 1) return 0;

		var i = wheelYSeen.length;
		if (i == 5) return dy;

		while (i--) if (wheelYSeen[i] == abs) break;
		if (i < 0) {
			wheelYSeen.push(abs);
			wheelYMin = Math.min(abs, wheelYMin);
		}

		if (wheelYSeen.length == 1) return dy > 0 ? 5 : -5;
		return dy / wheelYMin * 5;
	};

});
