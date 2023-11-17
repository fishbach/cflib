/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

import $          from '../dom.mjs';
import ajax       from '../net/ajax.mjs';
import rmi        from '../net/rmi.mjs';
import util       from '../util/util.mjs';
import logService from '../../services/logservice.mjs';

function logToServer(category, str)
{
	var file = 'javascript';
	var line = 0;
	try { util.__undefined(); } catch (e) { try {
		var parts = /.+\/[^\/]+\/[^\/]+.mjs:\d+.*\n.+\/[^\/]+\/[^\/]+.mjs:\d+.*\n.+\/([^\/]+\/[^\/]+.mjs):(\d+)/.exec(e.stack);
		file = parts[1];
		line = +parts[2];
	} catch (e2) {} }
	logService.log(file, line, category, str);
}

// ========================================================================

var NetUtil = function() {};
var netUtil = new NetUtil();

netUtil.logInfo     = function(str) { logToServer(4, str); };
netUtil.logWarn     = function(str) { logToServer(5, str); };
netUtil.logCritical = function(str) { logToServer(6, str); };

netUtil.getArrayBuffer = function(url, callback, scope, progress) {
	ajax.request('GET', url,
		function(status, buf, xhr) { callback.call(scope, status == 200 ? buf : null); }, null, null, null, 'arraybuffer', progress);
};

netUtil.initWaitingLayer = function(showWaitingFunc, hideWaitingFunc) {
	var waitingTimeout = null;
	var $backdrop = null;
	var keepWaiting = false;

	var disableKeys = function(e) {
		// IE 11 Bug:
		// It catches key events even from foreign iFrames.
		// This is ie a problem with the iFrame of Credit Card 3D Secure.
		if ($.all('iframe').length === 0) e.preventDefault();
	};

	rmi.ev.loading.bind(function(isLoading) {

		if (isLoading == 'on') {
			keepWaiting = true;
			if ($backdrop) {
				if (waitingTimeout) {
					clearTimeout(waitingTimeout);
					waitingTimeout = null;
					showWaitingFunc();
				}
				return;
			}
			isLoading = true;
		} else if (isLoading == 'off') {
			keepWaiting = false;
			isLoading = false;
		} else if (keepWaiting) return;

		if (isLoading && !$backdrop) {
			$backdrop = util.createBackdrop(false);
			$.unfocus();

			$(document).on('keydown', disableKeys);

			if (keepWaiting) {
				showWaitingFunc();
			} else {
				waitingTimeout = setTimeout(function() {
					waitingTimeout = null;
					showWaitingFunc();
				}, 500);
			}
		} else if (!isLoading && $backdrop) {
			if (waitingTimeout) {
				clearTimeout(waitingTimeout);
				waitingTimeout = null;
			} else {
				hideWaitingFunc();
			}
			$(document).off('keydown', disableKeys);
			$backdrop.remove();
			$backdrop = null;
		}
	});
};

export default netUtil;
