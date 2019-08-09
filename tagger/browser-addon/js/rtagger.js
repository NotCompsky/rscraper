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

var tags;
var reasons;
var flairs;

function process_from_reasons(d){
	reasons = d;
	main();
}

function process_from_tags(d){
	tags = d;
	main();
}

function process_from_flairs(d){
	flairs = d;
	main();
}

function main(){
    if (flairs === undefined  ||  reasons === undefined  ||  tags === undefined){
        // Not all fetch requests have been completed
        return;
    }
    for (var t of document.getElementsByClassName("author")){
        var s = t.classList[2];
        if (s[0] === 'm'){
            s = t.classList[3];
        }
        var tpls = flairs[s];
        // NOTE: Thread starter has additional (non-ID) tag in 2nd index.
        // TODO: Account for this
        if (tpls === undefined){
            continue;
        }
        for (var tpl of tpls){
            var tagstrtag = document.createElement("div");
            tagstrtag.innerText = reasons[tpl[0]] + " " + tpl[1];
            tagstrtag.style.background = tpl[2];
            tagstrtag.style.display = "inline";
            t.appendChild(tagstrtag);
        }
    }
}

chrome.storage.sync.get({
    reasons_url: "http://104.197.15.19:8080/api/reasons.json"
}, function(items) {
    fetch(items.reasons_url)
        .then(function(r){
            return r.json();
        })
        .then(function(json){
            process_from_reasons(json);
        })
        .catch(function(err){
            console.log(err);
        })
});

chrome.storage.sync.get({
    tags_url: "http://104.197.15.19:8080/api/tags.json"
}, function(items) {
    fetch(items.tags_url)
        .then(function(r){
            return r.json();
        })
        .then(function(json){
            process_from_reasons(json);
        })
        .catch(function(err){
            console.log(err);
        })
});

chrome.storage.sync.get({
    url: "http://104.197.15.19:8080/api/flairs/slurs/"
}, function(items) {
    fetch(items.url + userIds.join(","))
        .then(function(r){
            return r.json();
        })
        .then(function(json){
            process_from_flairs(json);
        })
        .catch(function(err){
            console.log(userIds + ": " + err);
        })
});



