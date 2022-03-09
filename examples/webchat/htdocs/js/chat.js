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

define([
	'cflib/dom',
	'cflib/net/ber',
	'cflib/net/rmi',
	'services/infoservice'
], function($, ber, rmi, infoService) {
	var Chat = function () {}
	var chat = new Chat();

	chat.start = function () {

		$("#sendButton").on('click', function () {
			var $input = $('#inputText');
			infoService.talk_String($input.val());
			$input.val('');
		});

		infoService.rsig.newMessage.register().bind(function (connectionId, text) {
			var $textarea = $('#chatTextArea');
			$textarea.val($textarea.val() + connectionId + ':\t' + text + '\n');
			$textarea.el.scrollTop = $textarea.el.scrollHeight - $textarea.height();
		});
	}

	return chat;
});
