/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

var Storage = function() {};
var storage = new Storage();

storage.get = function(name) {
    var rv = window.localStorage.getItem(name);
    if (rv === null) return window.sessionStorage.getItem(name);
    return rv;
};

storage.set = function(name, val, permanent) {
    if (permanent) window.localStorage.setItem(name, val);
    else           window.sessionStorage.setItem(name, val);
};

storage.remove = function(name) {
    window.localStorage.removeItem(name);
    window.sessionStorage.removeItem(name);
};

storage.clear = function(keepItems) {
    var i;
    var keep = [];
    if (keepItems) {
        if (!Array.isArray(keepItems)) keepItems = [keepItems];
        i = keepItems.length;
        while (i--) {
            var name = keepItems[i];
            var rv = window.localStorage.getItem(name);
            if (rv !== null) keep.push([true, name, rv]);
            else {
                rv = window.sessionStorage.getItem(name);
                if (rv !== null) keep.push([false, name, rv]);
            }
        }
    }
    window.localStorage.clear();
    window.sessionStorage.clear();
    i = keep.length;
    while (i--) {
        var el = keep[i];
        if (el[0]) window.localStorage  .setItem(el[1], el[2]);
        else       window.sessionStorage.setItem(el[1], el[2]);
    }
};

export default storage;
