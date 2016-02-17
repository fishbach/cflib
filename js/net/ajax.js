/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
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

	function createXHR() {
		var xhr;
		try { xhr = new XMLHttpRequest(); } catch (e1) {
			try { xhr = new ActiveXObject("Microsoft.XMLHTTP"); } catch (e2) {
				try { xhr = new ActiveXObject("Msxml2.XMLHTTP"); } catch (e3) {
					xhr = null;
				}
			}
		}
		return xhr;
	}

	function nop() {}

	// ========================================================================

	var Ajax = function() {};
	var ajax = new Ajax();

	if (createXHR() === null) {
		ajax.request = function(method, url, callback, content, headers, contentType, responseType, progressDown, progressUp) {
			callback();
		};
//	needed for CORS (Cross-origin resource sharing)
//	} else if (createXHR().withCredentials === undefined && typeof XDomainRequest !== "undefined") {
//		ajax.request = function(method, url, callback, content, headers, contentType) {
//			var xhr = new XDomainRequest();
//			xhr.open(method, url);
//			xhr.onerror = function() {
//				callback(400, 'IE error', xhr);	// xhr.responseText is always empty
//			};
//			xhr.ontimeout = xhr.onerror;
//			xhr.onload = function() {
//				callback(200, xhr.responseText, xhr);
//			};
//			try { xhr.send(content); } catch (e) {
//				callback();
//			}
//		};
	} else {
		ajax.request = function(method, url, callback, content, headers, contentType, responseType, progressDown, progressUp) {
			var xhr = createXHR();
			xhr.open(method, url, true);
			if (headers) {
				for (var h in headers) {
					xhr.setRequestHeader(h, headers[h]);
				}
			}
			if (content) xhr.setRequestHeader('Content-Type', contentType ? contentType : 'text/plain');
			if (responseType) xhr.responseType = responseType;
			xhr.onreadystatechange = function() {
				if (xhr.readyState != 4) return;
				xhr.onreadystatechange = nop;
				callback(xhr.status, responseType ? xhr.response : xhr.responseText, xhr);
			};
			if (progressDown) xhr.addEventListener('progress', progressDown, false);
			if (progressUp) xhr.upload.addEventListener('progress', progressUp, false);
			try { xhr.send(content); } catch (e) {
				callback();
			}
		};
	}

	return ajax;
});
