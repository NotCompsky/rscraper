/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "filter_subreddit.hpp"

#include "filter_init.hpp"


namespace filter_subreddit {

uint64_t* BLACKLIST_COUNT;
uint64_t* BLACKLIST_BODY;
uint64_t* WHITELIST_BODY;

void init(){
	filter::init("subreddit_count_bl", &BLACKLIST_COUNT);
	filter::init("subreddit_contents_bl",  &BLACKLIST_BODY);
	filter::init("subreddit_contents_wl",  &WHITELIST_BODY);
}

}
