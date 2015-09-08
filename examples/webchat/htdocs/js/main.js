require([
	'cflib/ext/jquery'
], function() { require([
	'cflib/net/ber',
	'cflib/net/rmi',
	'cflib/util/api',
	'cflib/util/storage',
	'cflib/util/url',
	'cflib/util/util',
	'services/infoservice'
], function(ber, rmi, api, Storage, url, util, infoService) { $(function() {

window.onerror = function() {
	util.logInfo("JS exception: " + Array.prototype.join.call(arguments, ', '));
	// It may happen that we have rubbish in Storage after an exception.
	api.resetStorage();
};

// server not reachable
api.ev.error.bind(function() {
	console.log("server not reachable");
});

// server might have been restarted with new version
api.ev.secureTokenLost.bind(function() {
	location.href = location.href;
});

rmi.ev.loading.bind(function(isLoading) { $('#loading').html(isLoading ? " (loading)" : ""); });
rmi.start();
infoService.test(function(val) {
	$('#msg').html(val);
});

window.ber = ber;
window.rmi = rmi;
window.util = util;
window.info = infoService;
window.plog = function() { console.log(arguments); };


}); }); });
