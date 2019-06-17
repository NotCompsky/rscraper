![Example 1](res/img/1.png)

![Example 2](res/img/2.png)

# Prerequisites

## Server

Relies on `rscraper-tagger` that is built in this project. See [root instructions](..) for details on installing or building that.

Aside from that, it just needs a server to run on, and that server must support C imports. In the examples you can find both a [Python flask server](server.py) and a [Go server](src/rtagger.go)

# Installation

You can either install the [Javascript script](rtagger.js) into the greasemonkey addon, or install the addon into your browser.

## AddOn

To install the addon, simply open the [rtagger.xpi](browser-addon/rtagger.xpi) file with your browser.

If you wish to build the `xpi` from source, navigate to the `browser-addon` directory and run `./build`.

# Usage

Your chosen must be running (on the port specified in [Javascript script](rtagger.js)) in order for the Javascript script to add tags to users.
