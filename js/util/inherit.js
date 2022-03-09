/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

	var Inherit = function() {};
	var inherit = new Inherit();

	inherit.Base = function() { inherit.Base.prototype.__init.apply(this, arguments); };
	inherit.Base.prototype.__init = function() {};
	inherit.setBase = function(cl, base) {
		var Prototype = function() {};
		Prototype.prototype = base.prototype;
		cl.prototype = new Prototype();
		cl.__super = base;
	};

	inherit.map = function(list, func) {
		if (!list) return [];
		var rv = [];
		for (var i = 0, l = list.length ; i < l ; ++i) rv.push(func(list[i]));
		return rv;
	};

	return inherit;
});
