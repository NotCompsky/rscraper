// Based on the example provided on https://developers.chrome.com/extensions/options

function save_options() {
    var port_n = document.getElementById('port_n').value;
    var is_https = document.getElementById('is_https').checked;
    var pth = document.getElementById('pth').value;
    chrome.storage.sync.set({
        port_n: port_n,
        is_https: is_https,
        pth: pth
    }, function() {
        // Update status to let user know options were saved.
        var status = document.getElementById('status');
        status.textContent = 'Options saved.';
        setTimeout(function() {
            status.textContent = '';
        }, 750);
    });
}

// Restores select box and checkbox state using the preferences stored in chrome.storage
function restore_options() {
  chrome.storage.sync.get({
    port_n: 8080,
    is_https: false,
    pth: "/"
  }, function(items) {
    document.getElementById('port_n').value = items.port_n;
    document.getElementById('is_https').checked = items.is_https;
    document.getElementById('pth').vlaue = items.pth;
  });
}
document.addEventListener('DOMContentLoaded', restore_options);
document.getElementById('save').addEventListener('click', save_options);
