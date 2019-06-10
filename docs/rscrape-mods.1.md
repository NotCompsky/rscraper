% RSCRAPE-MODS(1) RSCRAPE User Manual
% NotCompsky
% June 2019

# NAME

rscrape-mods - Collect comments from Reddit, and write to local SQL database.

# SYNOPSIS
rscrape-mods *MAX_DEPTH* [*SUBREDDITS*]

# ARGUMENTS

*MAX_DEPTH*
:   Maximum recursion depth. If 0, will only scrape the moderators of the specified subreddits. If larger, will crawl each moderator in turn to find their moderated subreddits, and add these subreddits to the queue to scrape.

*SUBREDDITS*
:   Names of the subreddit to scrape from, delineated by spaces.
    These subreddits must be present in the `subreddit` table of the `rscraper` MySQL database.

# EXAMPLES
    rscrape-mods 5 programming cpp python linux ubuntu

# SEE ALSO

*rscrape-cmnts(1)*, *rscraper(1)*
