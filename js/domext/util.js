/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

	var wheelYMin = Infinity;
	var wheelYSeen = [];

	// ========================================================================

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
