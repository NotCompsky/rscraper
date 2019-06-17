/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#ifndef __MYCU__
#define __MYCU__

#include <curl/curl.h>
#include <stdlib.h> // for malloc, realloc

#include "error_codes.hpp" // for myerr:*

#ifndef DEBUG
# define printf(...)
#else
# include <stdio.h> // for printf
#endif


/* Defined in redditcurl_utils.hpp */
namespace myrcu {
    extern void handler(int n);
}


namespace mycu {


CURL* curl;

struct curl_slist* HEADERS;

struct MemoryStruct {
    char* memory;
    size_t size;
    size_t n_allocated;
    void enlarge(size_t n);
};

struct MemoryStruct MEMORY;




size_t write_res_to_mem(void* content, size_t size, size_t n, void* buf){
    size_t total_size = size * n;
    struct MemoryStruct* mem = (struct MemoryStruct*)buf;
    
    if (mem->size + total_size  >  mem->n_allocated){
        mem->memory = (char*)realloc(mem->memory,  (mem->size + total_size + 1)*3/2);
        mem->n_allocated = (mem->size + total_size + 1)*3/2;
    }
    // Larger requests are not written in just one call
    
    if (!mem->memory)
        myrcu::handler(myerr::CANNOT_WRITE_RES);
    
    memcpy(mem->memory + mem->size,  content,  total_size);
    mem->size += total_size;
    mem->memory[mem->size] = 0;
    
    return total_size;
}


void request(const char* url){
    // Writes response contents to MEMORY
  #ifdef DEBUG
    printf("GET %s\n", url);
  #endif
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    MEMORY.size = 0; // 'Clear' last request
    
    if (curl_easy_perform(curl) != CURLE_OK)
        myrcu::handler(myerr::CURL_PERFORM);
}

void init(){
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl)
        myrcu::handler(myerr::CANNOT_INIT_CURL);
}


} // END namespace
#endif

/* Transition Regex
([^_A-Za-z:])(((HEADERS|MemoryStruct|MEMORY|write_res_to_memory|request)[^_A-Za-z])|curl_)
\1mycu::\2
*/
