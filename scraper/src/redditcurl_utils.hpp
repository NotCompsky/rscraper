/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef RSCRAPER_SCRAPER_REDDITCURL_UTILS_HPP
#define RSCRAPER_SCRAPER_REDDITCURL_UTILS_HPP

#include "rapidjson/document.h" // for rapidjson::Document


namespace myrcu {
	
constexpr int REDDIT_REQUEST_DELAY = 1;


void handler(int n);


void init_login(const char* fp);


void login();


void init(const char* fp);

bool try_again(rapidjson::Document& d);

void init_browser_curl();

void get_user_moderated_subs(const char* username);


} // END namespace
#endif

/* To convert id to name
curl 'https://oauth.reddit.com/api/info?id=tM_abcdef,tN_ghij' -H 'Authorization: bearer ...'
*/
