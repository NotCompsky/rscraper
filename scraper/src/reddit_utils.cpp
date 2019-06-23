/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "reddit_utils.hpp"

#include "error_codes.hpp" // for myerr:*


namespace myru {

uint64_t id2n_lower(const char* str){
    uint64_t n = 0;
    while (*str != 0){
        n *= (10 + 26);
        if (*str >= '0'  &&  *str <= '9')
            n += *str - '0';
        else
            n += *str - 'a' + 10;
        ++str;
    }
    return n;
}


} // END namespace
