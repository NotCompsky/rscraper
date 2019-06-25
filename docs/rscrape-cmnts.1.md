% RSCRAPE-CMNTS(1) RSCRAPER User Manual
% NotCompsky
% June 2019

# NAME

rscrape-cmnts - Collect comments from Reddit, and write to local SQL database.

# SYNOPSIS
rscrape-cmnts

# ENVIRONMENTAL VARIABLES

RSCRAPER_MYSQL_CFG
:    Required

RSCRAPER_REDDIT_CFG
:   Required

RSCRAPER_REGEX_FILE
:   Optional. If set, must point to a file containing a regex, which will be used to search the contents of comments - if matched, the comments will be entered into the database. See **REGEX FILE** below.

# REGEX FILE

## FORMAT

This regex can use named groups. These are not actually used by `boost::regex`, but are stripped out and instead used to populate `reason_matched` and relate the regex group indices to the reason.

Unnamed groups can be used too - they will just be assigned the `Unknown` reason.

## EXAMPLE

See [the example Python regex generator](../utils/comment_body_regexp.py).

# SEE ALSO

*rscraper(1)*
