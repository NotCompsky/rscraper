![Firefox (addon)](https://user-images.githubusercontent.com/30552567/60327591-2fe85980-9984-11e9-8461-273cb21eba1c.png)

![Chrome (userscript)](https://user-images.githubusercontent.com/30552567/60327286-6376b400-9983-11e9-8a4e-142a35ed38eb.png)

# Description

RTagger is a browser extension or userscript that will tag users on Reddit according to their post history.

You can point it to your own installation of the server software - see [Configuration](#Configuration) section below.

# Installation

Firefox users can install the addon by opening the `rtagger.xpi` file from the [releases page](https://github.com/NotCompsky/rscraper/releases) with their Firefox browser.

Users of other browsers can install the [userscript](rtagger.js) into their Greasemonkey/Tampermonkey/etc addon.

Creating an extension for Chrome isn't a priority, but I'm open to PRs.

# Configuration

By default, the addon and the userscript point towards a server at `http://104.197.15.19:8080`. This is only a proof of concept server - I won't be updating the database.

It is running on a Google cloud instance, so the IP is a subset of Google cloud's. That means it may be caught by some domain filters, such as uBlock Origin.

If you want customisation - such as more tagged subreddits, and different colours - you should edit your addon preferences (or userscript, whichever you installed) to make it point to another server.

To run your own instance of the `rscraper-tagger-server`, you would need to run the `rscraper-tagger` package. You would also need to point the server towards a MySQL/MariaDB database - for this, you'd need to install `rscraper-init` to (initialise it) and either `rscraper-scraper` or `rscraper-io` (to populate it). See the [root instructions](https://github.com/NotCompsky/rscraper) for how to do this.

# Building

If you wish to build the `xpi` from source, navigate to the `browser-addon` directory and run `./build`.

If you wish to build `rscraper-tagger` (the server executables), follow the instructions [here](../BUILDING.md).

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

# Related Projects

This module is similar to Reddit Pro Tools. Though while the latter is largely client-side code interacting with a central (closed-source?) server, this tagger has the minimal client-side processing, and almost all is done with native code on any specified server instance. And of course, while RPT is only available on Chrome, this can run on Firefox too. The pros of RPT: it filters based on karma count, the central server can serve everyone. Cons: [obfuscated analytics](https://bitbucket.org/feeling_impossible/reddit-pro-tools/src/master/analytics.js), far poorer performance (not very noticable for a few subreddits, but even then it's still worse than RTagger with almost 3000 tagged subreddits). In summary, I believe RPT suits the needs of most people fine; but this module might be useful as a supplementary tool for sleuths and moderators of larger subreddits, who would be dealing with larger lists of subreddits than the average user.
