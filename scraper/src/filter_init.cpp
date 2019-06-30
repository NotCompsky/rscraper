/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "filter_init.hpp"

#include "error_codes.hpp"

#include <compsky/mysql/query.hpp>

#include <stdlib.h> // for malloc, exit


extern MYSQL_RES* RES;
extern MYSQL_ROW ROW;


namespace filter {
    void init(const char* tblname,  uint64_t** list){
        size_t n_subreddits;
        
        compsky::mysql::query(&RES, "SELECT count(*) FROM ", tblname);
        
        while(compsky::mysql::assign_next_row(RES, &ROW, &n_subreddits));
        
        /* Pre-allocate memory for array to ensure continuity, as the lookup speed is important */
        uint64_t* dummy = (uint64_t*)malloc((n_subreddits + 1) * sizeof(uint64_t));
        if (dummy == nullptr)
            exit(myerr::OUT_OF_MEMORY);
        
        *list = dummy;
        
        compsky::mysql::query(&RES, "SELECT id FROM ", tblname);
        
        uint64_t subreddit_id;
        while(compsky::mysql::assign_next_row(RES, &ROW, &subreddit_id)){
            *(dummy++) = subreddit_id;
        }
        *dummy = 0;
    }
}
