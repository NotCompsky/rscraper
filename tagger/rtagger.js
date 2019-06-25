// ==UserScript==
// @name			rtagger
// @namespace		http://localhost/
// @description		Tag people according to https://github.com/NotCompsky/rscraper
// @copyright		2019+, Adam Gray (GPLv3.0)
// @include			https://www.reddit.com/*
// @include			https://old.reddit.com/*
// @include			https://new.reddit.com/*
// @version			0.0.1
// @grant			GM_xmlhttpRequest
// ==/UserScript==


var user_ids = [];
for (var t of document.getElementsByClassName("author")){
    var s = t.classList[2];
    if (s[0] == 'm'){
        s = t.classList[3];
    }
    user_ids.push(s);
}

function main(d){
    for (var t of document.getElementsByClassName("author")){
        var s = t.classList[2];
        if (s[0] == 'm'){
            s = t.classList[3];
        }
        var tpls = d[s];
        // NOTE: Thread starter has additional (non-ID) tag in 2nd index.
        // TODO: Account for this
        if (tpls === undefined){
            continue;
        }
        for (var tpl of tpls){
            tagstrtag = document.createElement("div");
            tagstrtag.innerHTML = tpl[1];
            tagstrtag.style.background = tpl[0];
            tagstrtag.style.display = "inline";
            t.appendChild(tagstrtag);
        }
    }
}

GM_xmlhttpRequest({
    method:     'GET',
    url:        'http://localhost:8080/' + user_ids.join(","),
    onload:     function(response){
                    main(JSON.parse(response.responseText));
                }
});