/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#ifndef __MYCU__
#define __MYCU__

#include <curl/curl.h>


/* Defined in redditcurl_utils.hpp */
namespace myrcu {
    extern void handler(int n);
}


namespace mycu {


extern CURL* curl;

extern struct curl_slist* HEADERS;

struct MemoryStruct {
    char* memory;
    size_t size;
    size_t n_allocated;
};

extern struct MemoryStruct MEMORY;


size_t write_res_to_mem(void* content, size_t size, size_t n, void* buf);

void request(const char* url);

void init();


} // END namespace
#endif
