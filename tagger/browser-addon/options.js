// Based on the example provided on https://developers.chrome.com/extensions/options

function save_options() {
    var flairs_url = document.getElementById("flairs_url").value;
    var reasons_url = document.getElementById("reasons_url").value;
	var tags_url = document.getElementById("tags_url").value;
	var user_info_url = document.getElementById("user_info_url").value;
    chrome.storage.sync.set({
        flairs_url: flairs_url,
        reasons_url: reasons_url,
        tags_url: tags_url,
		user_info_url: user_info_url
    }, function() {
        // Update status to let user know options were saved.
        var status = document.getElementById("status");
        status.textContent = "Options saved.";
        setTimeout(function() {
            status.textContent = "";
        }, 750);
    });
}

// Restores select box and checkbox state using the preferences stored in chrome.storage
function restore_options() {
  chrome.storage.sync.get({
    flairs_url: "https://104.197.15.19:8080/api/flairs/slurs/",
    reasons_url: "https://104.197.15.19:8080/api/reasons.json",
    tags_url: "https://104.197.15.19:8080/api/tags.json",
	user_info_url: "https://104.197.15.19:8080/u/#"
  }, function(items) {
    document.getElementById("flairs_url").value = items.flairs_url;
    document.getElementById("reasons_url").value = items.reasons_url;
    document.getElementById("tags_url").value = items.tags_url;
	document.getElementById("user_info_url").value = items.user_info_url;
  });
}
document.addEventListener("DOMContentLoaded", restore_options);
document.getElementById("save").addEventListener("click", save_options);
