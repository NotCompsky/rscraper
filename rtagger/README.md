Contains examples of how to use rtagger in a server.

# General

src/rtagger.js should be installed into your browser's Greasemonkey (or equivalent) addon.

AUTHORISATION_FILE_PATH must be specified at program launch; it must be the file path of a file containing the url, username, and password to your MySQL database, each on their own line, and in that order.

Note that Windows may have issues since by default their newlines consist of two characters (\r\n). This program does not account for it, so use a text editor such as the 2018+ Notepad or Notepad++.

# Python

# Golang

I gave up trying to get relative linking to work. As such, you will have to edit the line beginning "#cgo LDFLAGS: " to replace ${PWD} with this file's directory path.
