require([
	'cflib/ext/jquery'
], function() { require([
	'cflib/util/api',
	'cflib/util/storage',
	'cflib/util/url',
	'cflib/util/util'
], function(api, Storage, url, util) { $(function() {

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

console.log('init ...');

}); }); });
