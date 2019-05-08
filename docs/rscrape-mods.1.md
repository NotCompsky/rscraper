% RSCRAPE-MODS(1) RSCRAPE User Manual
% NotCompsky
% 13 April 2019

# NAME

rscrape-mods - Collect comments from Reddit, and write to local SQL database.

# SYNOPSIS
rscrape-mods *MYSQL_CONFIG_FILE* *REDDIT_CONFIG_FILE* *MAX_DEPTH* [*SUBREDDITS*]

# ARGUMENTS

*MYSQL CONFIG FILE*
:   See *rscrape-cmnts(1)*

*REDDIT CONFIG FILE*
:   See *rscrape-cmnts(1)*

*MAX_DEPTH*
:   Maximum recursion depth. If 0, will only scrape the moderators of the specified subreddits. If larger, will crawl each moderator in turn to find their moderated subreddits, and add these subreddits to the queue to scrape.

*SUBREDDITS*
:   Names of the subreddit to scrape from, delineated by spaces.
    These subreddits must be present in the `subreddit` table of the `rscraper` MySQL database.

# EXAMPLES
    rscrape-mods ~/.config/rscraper++/mysql.cfg ~/.config/rscraper++/reddit.cfg 5 programming cpp python linux ubuntu

# SEE ALSO

*rscrape-cmnts(1)*
