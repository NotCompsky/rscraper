![Firefox (addon)](https://user-images.githubusercontent.com/30552567/60327591-2fe85980-9984-11e9-8461-273cb21eba1c.png)

![Chrome (userscript)](https://user-images.githubusercontent.com/30552567/60327286-6376b400-9983-11e9-8a4e-142a35ed38eb.png)

# Installation

Firefox users can install the addon by opening the `rtagger.xpi` file from the releases page with their Firefox browser.

Users of other browsers can install the [userscript](rtagger.js) into their Greasemonkey/Tampermonkey/etc addon.

Creating an extension for Chrome isn't a priority, but I'm open to PRs.

# Configuration

By default, the addon and the userscript point towards a server at `http://localhost:8080`. To run such a server, you would need to install the `rscraper-tagger` package (see [root instructions](https://github.com/NotCompsky/rscraper) for how to do that).

You can, however, point it towards any server - local or remote - so long as it sends the expected JSON response. The addon allows you to specify the port number, domain name, protocol, and path (that is, the `/this/is/the.path` of `https://www.foo.bar/this/is/the.path`) of the server. If you are using the userscript, you would have to edit it yourself to change these.

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
