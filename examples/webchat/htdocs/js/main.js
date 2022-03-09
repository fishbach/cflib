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

require([
	'cflib/domext/util'
], function() { require([
	'cflib/dom',
	'cflib/net/netutil',
	'cflib/net/rmi',
	'cflib/util/storage',
	'chat'
], function(
	$, netUtil, rmi, storage, chat
) { $.start(function() {

window.onerror = function() {
	netUtil.logInfo("JS exception: " + Array.prototype.join.call(arguments, ', '));
	// It may happen that we have rubbish in storage after an exception.
	storage.clear();
};

// server might have been restarted with new version
rmi.ev.identityReset.bind(function() {
	location.href = location.href;
});

rmi.ev.loading.bind(function(isLoading) { $('#loading').html(isLoading ? " (loading)" : ""); });

rmi.start();
chat.start();

}); }); });
