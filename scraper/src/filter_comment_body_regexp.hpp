/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifdef USE_BOOST_REGEX
#ifndef RSCRAPER_SCRAPER_FILTER_COMMENT_BODY_REGEXP_HPP
#define RSCRAPER_SCRAPER_FILTER_COMMENT_BODY_REGEXP_HPP

#include <inttypes.h>
#include <vector>


namespace filter_comment_body {

extern std::vector<int> groupindx2reason;
extern std::vector<bool> record_contents;

extern std::vector<std::vector<uint64_t>> SUBREDDIT_WHITELISTS;
extern std::vector<std::vector<uint64_t>> SUBREDDIT_BLACKLISTS;

void init();

}


#endif
#endif
