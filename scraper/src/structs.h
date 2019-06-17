/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include <inttypes.h> // for uint64_t

struct cmnt_meta {
    const char* author_name;
    const char* subreddit_name;
    
    // ID comparison should be faster for filtering based on exact matches
    const uint64_t author_id;
    const uint64_t subreddit_id;
};
