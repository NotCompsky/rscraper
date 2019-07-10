% RSCRAPE-TAGGER-SERVER(1) RSCRAPER User Manual
% NotCompsky
% June 2019

# NAME

rscraper-tagger-server - Server.

# SYNOPSIS

## Golang

rscraper-tagger-server [*OPTIONS*]

## Python

rscraper-tagger-server.py [*OPTIONS*]

# OPTIONS

*-p* NUMBER
:   Port number
*-t* STRING
:   SQL condition on *t* (tag). Either empty, or must begin with `AND `. E.g. `AND t.id=3`, `AND t.name IN ('foo','bar')`.
*-m* STRING
:   SQL condition on *m* (reason_matched), as above.

# SEE ALSO

*rscraper(1)*
