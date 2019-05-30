#ifndef __MYCU__
#define __MYCU__

#include <curl/curl.h>
#ifdef DEBUG
  #include <stdio.h> // for printf
#endif
#include <stdlib.h> // for malloc, realloc

#include "error_codes.hpp" // for myerr:*



#ifdef DEBUG
  #define PRINTF(...) printf(__VA_ARGS__);
#else
  #define PRINTF(...) ;
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
