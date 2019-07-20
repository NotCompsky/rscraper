/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


var userIds = [];
for (var t of document.getElementsByClassName("author")){
    var s = t.classList[2];
    if (s[0] == 'm'){
        s = t.classList[3];
    }
    userIds.push(s);
}

function main(d){
    for (var t of document.getElementsByClassName("author")){
        var s = t.classList[2];
        if (s[0] === 'm'){
            s = t.classList[3];
        }
        var tpls = d[s];
        // NOTE: Thread starter has additional (non-ID) tag in 2nd index.
        // TODO: Account for this
        if (tpls === undefined){
            continue;
        }
        for (var tpl of tpls){
            var tagstrtag = document.createElement("div");
            tagstrtag.innerText = tpl[1];
            tagstrtag.style.background = tpl[0];
            tagstrtag.style.display = "inline";
            t.appendChild(tagstrtag);
        }
    }
}

chrome.storage.sync.get({
    url: "http://104.197.15.19:8080/rtagger/"
}, function(items) {
    fetch(items.url + userIds.join(","))
        .then(function(r){
            return r.json();
        })
        .then(function(json){
            main(json);
        })
        .catch(function(err){
            console.log(userIds + ": " + err);
        };)
});



