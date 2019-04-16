#include <curl/curl.h>
#include <stdio.h> // for printf // TMP
#include <stdlib.h> // for free, malloc, realloc
#include <string.h> // for memcpy

#define ERR_CANNOT_INIT_CURL 2
#define ERR_CANNOT_WRITE_RES 3
#define ERR_CURL_PERFORM 4


const char* USER_AGENT = "rscraper++ 0.0.1-dev0";
CURL* curl;
const char* PARAMS = "?limit=2048&sort=new&raw_json=1";
const char* AUTH_HEADER_PREFIX = "Authorization: bearer ";
const char* TOKEN_FMT = "XXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXXXX";
char* AUTH_HEADER;


struct curl_slist* HEADERS;

struct MemoryStruct {
    char* memory;
    size_t size;
};

struct MemoryStruct MEMORY;

void handler(int n){
    free(MEMORY.memory);
    free(AUTH_HEADER);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    exit(n);
}

size_t write_res_to_mem(void* content, size_t size, size_t n, void* buf){
    size_t total_size = size * n;
    struct MemoryStruct* mem = (struct MemoryStruct*)buf;
    
    mem->memory = (char*)realloc(mem->memory,  mem->size + total_size + 1);
    // Larger requests are not written in just one call
    if (!mem->memory)
        handler(ERR_CANNOT_WRITE_RES);
    
    memcpy(mem->memory + mem->size,  content,  total_size);
    mem->size += total_size;
    mem->memory[mem->size] = 0;
    
    return total_size;
}


int request(const char* reqtype, const char* base_url){
    // Writes response contents to MEMORY
    
    
    char url[strlen(base_url) + strlen(PARAMS) + 1];
    int i;
    
    i = 0;
    memcpy(url + i,  base_url,  strlen(base_url));
    i += strlen(base_url);
    
    memcpy(url + i,  PARAMS,  strlen(PARAMS));
    i += strlen(PARAMS);
    
    url[i++] = 0;
    
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, reqtype);
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_res_to_mem);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&MEMORY);
    
    
    if (curl_easy_perform(curl) != CURLE_OK)
        handler(ERR_CURL_PERFORM);
    
    printf(MEMORY.memory); // tmp
}

void login(const char* usr, const char* pwd, const char* key_and_secret){
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, long CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, key_and_secret);
    
    
    const char* a = "grant_type=password&password=";
    const char* b = "&username=";
    
    char postdata[strlen(a) + strlen(pwd) + strlen(b) + strlen(usr) + 1];
    
    int i = 0;
    
    memcpy(postdata + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(postdata + i,  pwd,  strlen(pwd));
    i += strlen(pwd);
    
    memcpy(postdata + i,  b,  strlen(b));
    i += strlen(b);
    
    memcpy(postdata + i,  usr,  strlen(usr));
    i += strlen(usr);
    
    postdata[i] = 0;
    
    
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
    
    
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.reddit.com/api/v1/access_token");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_res_to_mem);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&MEMORY);
    
    auto rc = curl_easy_perform(curl);
    
    if (rc != CURLE_OK)
        handler(ERR_CURL_PERFORM);
    
    
    // Result is in format
    // {"access_token": "XXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXXXX", "token_type": "bearer", "expires_in": 3600, "scope": "*"}
    
    AUTH_HEADER = (char*)realloc(AUTH_HEADER,  strlen(AUTH_HEADER_PREFIX) + strlen(TOKEN_FMT) + 1);
    
    i = 0;
    memcpy(AUTH_HEADER + i,  AUTH_HEADER_PREFIX,  strlen(AUTH_HEADER_PREFIX));
    i += strlen(AUTH_HEADER_PREFIX);
    
    memcpy(AUTH_HEADER + i,  MEMORY.memory + strlen("{\"access_token\": \""),  strlen(TOKEN_FMT));
    i += strlen(TOKEN_FMT);
    
    AUTH_HEADER[i] = 0;
    
    
    printf("AUTH_HEADER: %s\n", AUTH_HEADER);
    
    
    curl_slist_append(HEADERS, AUTH_HEADER);
}

int main(const int argc, const char* argv[]){
    MEMORY.memory = (char*)malloc(0);
    AUTH_HEADER = (char*)malloc(0);
    
    int i;
    
    const char* usr = argv[1];
    const char* pwd = argv[2];
    const char* authstr = argv[3];
    
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl)
        handler(ERR_CANNOT_INIT_CURL);
    
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    
    curl_slist_append(HEADERS, "Accept-Encoding: gzip, deflate");
    curl_slist_append(HEADERS, "Accept: */*");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, HEADERS);
    
    login(usr, pwd, authstr);
    
    handler(0);
}
