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

# PERFORMANCE

By far the largest import you will have will almost certainly be `user2subreddit_cmnt_count.csv`. It should handle e.g. a 200MB CSV import in under a minute, though this will increase (potentially a lot) if there are other processes accessing the rscraper database.

# EXAMPLES

*rscraper-import *.csv*
:   Import all recognisably-named csv files from the current directory, in the correct order.
    Note that the order of imports does matter if you run *rscraper-import* for individual files. For instance, tag2category should only be imported after both tag and category tables have been, if the goal is to relate the tags from tag.csv to the categories from category.csv.

# SEE ALSO

*rscraper(1)*, *rscraper-export(1)*
