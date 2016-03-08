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
			infoService.talk_String($("#inputText").val());
			document.getElementById("inputText").value = "";
		});

		infoService.rsig.newMessage.register().bind(function (connectionId, text) {
			var chatTextArea = $("#chatTextArea");
			chatTextArea.html(chatTextArea.val() + connectionId + ":\t" + text + "\n");
			document.getElementById("chatTextArea").scrollTop = document.getElementById("chatTextArea").scrollHeight;
		});
	}

	return chat;
});
