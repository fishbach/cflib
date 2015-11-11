require([
	'cflib/domext/util'
], function() { require([
	'cflib/dom',
	'cflib/net/netutil',
	'cflib/net/rmi',
	'cflib/util/storage',
	'cflib/util/util',
	'services/infoservice'
], function($, netUtil, rmi, storage, util, infoService) { $.start(function() {

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
infoService.test(function(val) {
	$('#msg').html(val);
});

rmi.register(3, function(data) { console.log('sig: ', data); });

window.rmi = rmi;
window.info = infoService;
window.plog = function() { console.log(arguments); };
window.util = util;
window.$ = $;

}); }); });
