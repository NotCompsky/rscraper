% RSCRAPER-INIT(1) RSCRAPER User Manual
% NotCompsky
% June 2019

# NAME

rscraper-init - Interactive tool for initialising the MySQL database for use by rscraper utilities.

# SYNOPSIS

rscraper-init

# ABOUT TABLES

## CATEGORY

Categories are what we group tags in, for the purposes of generating user tags as an average of the subreddit tags in each category.
For instance, /r/python, /r/cpp and /r/programming might be tagged 'coding', and /r/sewing might be tagged 'sewing', but we could usefully tag someone with both 'hobby: coding' and 'language: cpp'.
Currently works best (i.e. output makes the most sense) if at most one tag per category is assigned to each subreddit.

## MODERATOR

`permissions`
:   Integer whose bits represent each possible permission. This is only nonzero if we have scraped the moderators of the subreddit, rather than the moderated subreddits of a user.

`last_updated`
:    Update with `last_updated=now()`

`rank`
:   Starts at `1`. A value of `0` means it was not set.

# SEE ALSO

*rscraper(1)*
