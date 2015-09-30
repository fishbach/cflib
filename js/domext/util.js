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

	var animEls     = [];
	var animTimer   = null;
	var wheelYMin   = Infinity;
	var wheelYMax   = 0;
	var wheelYStart = 0;
	var wheelYCount = 0;
	var wheelYTrans = null;

	function setAnimProps(el, props)
	{
		for (var p in props) {
			if (p == 'scrollTop') {
				window.scrollTo($.scrollPos()[0], props[p]);
			} else {
				el.css(p, props[p] + 'px');
			}
		}
	}

	function getAnimProps(el, props)
	{
		var rv = {};
		for (var p in props) {
			if (p == 'scrollTop') {
				rv[p] = $.scrollPos()[1];
			} else {
				var val = el.css(p);
				rv[p] = !val ? 0 : parseInt(val, 10);
			}
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
					for (var prop in sProp) nProp[prop] = eProp[prop] * p + sProp[prop] * (1 - p);
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
			this.anim.queue.push([prop, ms, finishFunc, easing]);
			return this;
		}

		this.anim = {};
		this.anim.queue = [];
		this.anim.prop = prop;
		this.anim.duration = ms;
		this.anim.finish = finishFunc;
		this.anim.easing = easing;
		this.anim.start = new Date().getTime();
		this.anim.startProp = getAnimProps(this, prop);
		animEls.push(this);
		if (!animTimer) animTimer = setInterval(animStep, 13);
		return this;
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
				.animate({ left: -5 }, 40)
				.animate({ left:  5 }, 80)
				.animate({ left:  0 }, 40, !i ? finished : undefined);
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
		console.log(e.deltaY, e.deltaMode, wheelYTrans ? wheelYTrans(e.deltaY) : null);
		var dy = e.deltaY;
		if (wheelYTrans) return wheelYTrans(dy);

		if (Math.abs(dy) < 1) return 0;
		++wheelYCount;
		if (!wheelYStart) wheelYStart = new Date().getTime();
		if (new Date().getTime() - wheelYStart < 1000 || wheelYCount <= 5) {
			wheelYMin = Math.min(Math.abs(dy), wheelYMin);
			wheelYMax = Math.max(Math.abs(dy), wheelYMax);
			return dy > 0 ? 5 : -5;
		}

		/*jshint smarttabs:true */
		console.log('mima: ', wheelYMin, wheelYMax);
		wheelYTrans =
			wheelYMin == wheelYMax    ? function(x) { return x > 0 ? 5 : -5; } :
			wheelYMax / wheelYMin > 3 ? function(x) { return x; } :
			                            function(x) { return x / wheelYMin * 5; };

		return wheelYTrans(dy);
	};

});
