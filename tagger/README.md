![Firefox (addon)](https://user-images.githubusercontent.com/30552567/60322807-48eb0d80-9978-11e9-94a4-d4e92e38bf25.png)

![Chrome (userscript)](https://user-images.githubusercontent.com/30552567/60327286-6376b400-9983-11e9-8a4e-142a35ed38eb.png)

# Installation

Firefox users can install the addon by opening the `rtagger.xpi` file from the releases page with their Firefox browser.

Users of other browsers can install the [Javascript script](rtagger.js) into their Greasemonkey/Tampermonkey/etc addon.

Creating an extension for Chrome isn't a priority, but I'm open to PRs.

# Building

If you wish to build the `xpi` from source, navigate to the `browser-addon` directory and run `./build`.

# Usage

Only works on the old Reddit style - you can view Reddit with this by default in your preferences on Reddit.

Requires a server running on the address and port number specified by your addon options or within the Greasemonkey script (default is `8080`).

Usually this will be a local server instance, in which case you need to install the entire `rscraper` package yourself (see [root](..)). In the examples you can find both a [Python flask server](server.py) and a [Go server](src/server.go), either of which should work.

However, you can also set it up to use server of your choosing at any given url - although in that case you would need to edit the addon by hand (see `editing` section below). In that case, the addon or Greasemonkey script is all that you will need.

# Editing

## AddOn

If you are not using a `localhost` server, you will need to edit the [manifest](browser-addon/manifest.json) and add your server's url to the permissions.

Then edit [rtagger.js](browser-addon/js/rtagger.js), and replace `url += "://localhost:";` with the desired address.

Then rebuild with `./build`, and reinstall.

## Greasemonkey Script

Just edit the request url.
