% RSCRAPER(1) RSCRAPER User Manual
% NotCompsky
% June 2019

# NAME

rscrape - Family of utilities for scraping from Reddit and utilising the scraped data.

# USAGE

Prior to using the programs, the relevant environmental variables must be set.

# ENVIRONMENTAL VARIABLES

*RSCRAPER_MYSQL_CFG*
:   File path of the MySQL config file. Use *rscraper-init(1)* to initialse this, as it is very particular about the format.

*RSCRAPER_REDDIT_CFG*
:   File path of the Reddit user authorisation. Use *rscraper-init-scraper(1)* to create this, as it is also particular about the format.

*RSCRAPER_REGEX_FILE*
:   Only used by *rscrape-cmnts(1)*, and not required for it. Path of a file containing a regex against which to match comment contents.

# SEE ALSO

*rscrape-cmnts(1)*,
*rscraper-hub(1)*
*rscraped-reason(1)*,
*rscraped-tagged-subs(1)*,
*rscraper-export(1)*,
*rscraper-id2str(1)*,
*rscraper-import(1)*,
*rscraper-init-scraper(1)*,
*rscraper-init(1)*,
*rscraper-str2id(1)*,
*rscraper-tagger-server(1)*,
