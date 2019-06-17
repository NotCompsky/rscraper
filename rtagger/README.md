![Example 1](res/img/1.png)

![Example 2](res/img/2.png)

# Installation

You can either install the [Javascript script](rtagger.js) into the greasemonkey addon, or open [the addon](browser-addon/rtagger.xpi) with your browser and install that.

## AddOn

To install the addon, simply open the [rtagger.xpi](browser-addon/rtagger.xpi) file with your browser.

If you wish to build the `xpi` from source, navigate to the `browser-addon` directory and run `./build`.

# Usage

Requires a server running on the address and port number specified by your addon options or within the Greasemonkey script (default is `8080`).

Usually this will be a local server instance, in which case you need to install the entire `rscraper` package yourself (see [root](..)). In the examples you can find both a [Python flask server](server.py) and a [Go server](src/rtagger.go), either of which should work.

However, you can also set it up to use server of your choosing at any given url - although in that case you would need to edit the addon by hand (see `editing` section below). In that case, the addon or Greasemonkey script is all that you will need.

# Editing

## AddOn

If you are not using a `localhost` server, you will need to edit the [manifest](browser-addon/manifest.json) and add your server's url to the permissions.

Then edit [rtagger.js](browser-addon/js/rtagger.js), and replace `url += "://localhost:";` with the desired address.

Then rebuild with `./build`, and reinstall.

## Greasemonkey Script

Just edit the request url.
