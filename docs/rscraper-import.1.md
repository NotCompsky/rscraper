% RSCRAPER(1) RSCRAPER User Manual
% NotCompsky
% June 2019

# NAME

rscraper-import - Export from the rscraper database in a format that can be imported by **rscraper-import(1)**.

# SYNOPSIS

rscrape-mods [*OPTIONS*] [*FILES*]

# OPTIONS

*-f*
:   Force import, i.e. if there is a conflict, overwrite existing data with the imported data.

# ARGUMENTS

*FILES*
:   Name(s) of tables to import with '.csv' appended. Tables include `user`, `subreddit`, `tag`, `subreddit2tag`, `tag2category`, `category`, and `user2subreddit_cmnt_count`.

# SEE ALSO

*rscraper(1)*, *rscraper-export(1)*
