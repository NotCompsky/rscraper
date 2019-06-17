let user_ids = [];
for (var t of document.getElementsByClassName("author")){
    user_ids.push(t.classList[2]);
}

function main(d){
    for (var t of document.getElementsByClassName("author")){
        var tpls = d[t.classList[2]];
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

chrome.storage.sync.get({
    port_n: 8080,
    is_https: false,
    pth: "/"
}, function(items) {
    var url = "http";
    if (items.is_https){
        url += "s";
    }
    url += "://localhost:";
    url += items.port_n;
    url += items.pth;
    url += user_ids.join(",");
    
    console.log("Fetching " + url);
    
    fetch(url)
        .then(function(r){
            return r.json();
        })
        .then(function(json){
            main(json);
        })
        .catch(function(err){
            console.log(err);
        })
});



