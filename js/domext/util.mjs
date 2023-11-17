/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

import $ from '../dom.mjs';

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
