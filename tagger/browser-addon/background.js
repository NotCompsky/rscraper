chrome.runtime.onMessage.addListener(
	function(request, sender, sendResponse) {
		var reasons;
		var tags;
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
			sendResponse({
				flairs: flairs,
				tags: tags,
				reasons: reasons
			});
		}

		chrome.storage.sync.get({
			reasons_url: "https://104.197.15.19:8080/api/reasons.json"
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
			tags_url: "https://104.197.15.19:8080/api/tags.json"
		}, function(items) {
			fetch(items.tags_url)
				.then(function(r){
					return r.json();
				})
				.then(function(json){
					process_from_tags(json);
				})
				.catch(function(err){
					console.log(err);
				})
		});

		chrome.storage.sync.get({
			flairs_url: "https://104.197.15.19:8080/api/flairs/slurs/"
		}, function(items) {
			fetch(items.flairs_url + request.user_ids.join(","))
				.then(function(r){
					return r.json();
				})
				.then(function(json){
					process_from_flairs(json);
				})
				.catch(function(err){
					console.log(request.user_ids + ": " + err);
				})
		});
	}
);
