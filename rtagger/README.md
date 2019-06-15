![Example 1](res/img/1.png)

![Example 2](res/img/2.png)

# Installation

Install the [Javascript script](rtagger/src/rtagger.js) into greasemonkey.

# Usage

[server.py](rtagger/server.py) must be running (on the port specified in [Javascript script](rtagger/src/rtagger.js)) in order to add tags to users.

# Requirements

## Browser Addons

    * GreaseMonkey or similar

## Server

To use the Python server example:

    * flask

# General

src/rtagger.js should be installed into your browser's Greasemonkey (or equivalent) addon.

AUTHORISATION_FILE_PATH must be specified at program launch; it must be the file path of a file containing the url, username, and password to your MySQL database, each on their own line, and in that order.

Note that Windows may have issues since by default their newlines consist of two characters (\r\n). This program does not account for it, so use a text editor such as the 2018+ Notepad or Notepad++.

# Python

# Golang

I gave up trying to get relative linking to work. As such, you will have to edit the line beginning "#cgo LDFLAGS: " to replace ${PWD} with this file's directory path.
