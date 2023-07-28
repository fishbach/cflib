/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
