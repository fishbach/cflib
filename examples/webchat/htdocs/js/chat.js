/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
