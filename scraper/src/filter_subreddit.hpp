/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#ifndef RSCRAPER_SCRAPER_FILTER_SUBREDDIT_HPP
#define RSCRAPER_SCRAPER_FILTER_SUBREDDIT_HPP

#include <inttypes.h> // for uint64_t


namespace filter_subreddit {

extern uint64_t* BLACKLIST_COUNT;
extern uint64_t* BLACKLIST_BODY;
extern uint64_t* WHITELIST_BODY;
void init();

}

#endif
