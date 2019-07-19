/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#ifndef RSCRAPER_SCRAPER_ERROR_CODES_HPP
#define RSCRAPER_SCRAPER_ERROR_CODES_HPP

namespace myerr {
enum {
	NONE,
	UNKNOWN,
	CANNOT_INIT_CURL,
	CANNOT_WRITE_RES,
	CURL_PERFORM,
	CANNOT_SET_PROXY,
	INVALID_PJ,
	JSON_PARSING,
	BAD_ARGUMENT,
	UNACCOUNTED_FOR_SERVER_CODE,
	SUBREDDIT_NOT_IN_DB,
	UNAUTHORISED,
	UNSUPPORTED_GRANT_TYPE,
	OUT_OF_MEMORY,
	IMPOSSIBLE // Should never happen
};
}
#endif
