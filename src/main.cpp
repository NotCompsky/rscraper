#include <curl/curl.h>
#include <stdio.h> // for printf // TMP
#include <stdlib.h> // for free, malloc, realloc
#include <string.h> // for memcpy
#include <unistd.h> // for sleep

#define ERR_CANNOT_INIT_CURL 2
#define ERR_CANNOT_WRITE_RES 3
#define ERR_CURL_PERFORM 4
#define ERR_CANNOT_SET_PROXY 5
#define ERR_CANNOT_SET_BEARER_TOKEN 6


const char* USER_AGENT = "rscraper++:0.0.1-dev0 (by /u/Compsky)";
CURL* curl;
const char* PARAMS = "?limit=2048&sort=new&raw_json=1";
const char* TOKEN_FMT = "XXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXXXX";

char OATH_BEARER_TOKEN[36 + 1]; //[strlen(TOKEN_FMT) + 1];

#ifdef DEBUG
const char* API_SUBMISSION_URL_PREFIX = "localhost:8000/comments/";
#else
const char* API_SUBMISSION_URL_PREFIX = "https://oauth.reddit.com/comments/";
#endif


struct curl_slist* HEADERS;

struct MemoryStruct {
    char* memory;
    size_t size;
};

struct MemoryStruct MEMORY;

static void
print_cookies(CURL *curl) // TMP
{
  CURLcode res;
  struct curl_slist *cookies;
  struct curl_slist *nc;
  int i;
 
  printf("Cookies, curl knows:\n");
  res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
  if(res != CURLE_OK) {
    fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n",
            curl_easy_strerror(res));
    exit(1);
  }
  nc = cookies;
  i = 1;
  while(nc) {
    printf("[%d]: %s\n", i, nc->data);
    nc = nc->next;
    i++;
  }
  if(i == 1) {
    printf("(none)\n");
  }
  curl_slist_free_all(cookies);
}

void handler(int n){
    free(MEMORY.memory);
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


int request(const char* reqtype, const char* url){
    // Writes response contents to MEMORY
    
    /*
    char url[strlen(base_url) + strlen(PARAMS) + 1];
    int i;
    
    i = 0;
    memcpy(url + i,  base_url,  strlen(base_url));
    i += strlen(base_url);
    
    memcpy(url + i,  PARAMS,  strlen(PARAMS));
    i += strlen(PARAMS);
    
    url[i++] = 0;
    */
    
    printf("%s %s\n", reqtype, url);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, reqtype);
    
    
    if (curl_easy_perform(curl) != CURLE_OK)
        handler(ERR_CURL_PERFORM);
    
    printf("MEMORY.memory: %s\n", MEMORY.memory); // TMP
}

void login(const char* usr, const char* pwd, const char* key_and_secret){
    int i;
    
    
    // TODO: Copy cookies to global curl object?
    
    
    /* Need a seperate curl object for first request, as it uses different authentification */
    CURL* curl_local = curl_easy_init();
    if (!curl_local)
        handler(ERR_CANNOT_INIT_CURL);
    
    curl_easy_setopt(curl_local, CURLOPT_USERAGENT, USER_AGENT);
    
    
    /*
    struct curl_slist* headers;
    
    curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
    curl_slist_append(headers, "Accept: **");
    curl_easy_setopt(curl_local, CURLOPT_HTTPHEADER, headers);
    */
    
    
    curl_easy_setopt(curl_local, CURLOPT_COOKIEFILE, ""); // Init cookie engine
    
    curl_easy_setopt(curl_local, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl_local, CURLOPT_USERPWD, key_and_secret);
    
    
    const char* a = "grant_type=password&password=";
    const char* b = "&username=";
    
    char postdata[strlen(a) + strlen(pwd) + strlen(b) + strlen(usr) + 1];
    
    i = 0;
    
    memcpy(postdata + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(postdata + i,  pwd,  strlen(pwd));
    i += strlen(pwd);
    
    memcpy(postdata + i,  b,  strlen(b));
    i += strlen(b);
    
    memcpy(postdata + i,  usr,  strlen(usr));
    i += strlen(usr);
    
    postdata[i] = 0;
    
    
    curl_easy_setopt(curl_local, CURLOPT_POSTFIELDS, postdata);
    
    
  #ifdef DEBUG
    curl_easy_setopt(curl_local, CURLOPT_URL, "https://www.reddit.com/api/v1/access_token");
  #else
    curl_easy_setopt(curl_local, CURLOPT_URL, "https://www.reddit.com/api/v1/access_token");
  #endif
    
    curl_easy_setopt(curl_local, CURLOPT_CUSTOMREQUEST, "POST");
    
    curl_easy_setopt(curl_local, CURLOPT_WRITEFUNCTION, write_res_to_mem);
    curl_easy_setopt(curl_local, CURLOPT_WRITEDATA, (void *)&MEMORY);
    
    auto rc = curl_easy_perform(curl_local);
    
    if (rc != CURLE_OK)
        handler(ERR_CURL_PERFORM);
    
    
    // Result is in format
    // {"access_token": "XXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXXXX", "token_type": "bearer", "expires_in": 3600, "scope": "*"}
    
    memcpy(OATH_BEARER_TOKEN,  MEMORY.memory + strlen("{\"access_token\": \""),  strlen(TOKEN_FMT));
    OATH_BEARER_TOKEN[strlen(TOKEN_FMT)] = 0;
    
    MEMORY.size = 0; // No longer need contents of request
    
    
    print_cookies(curl_local); // TMP
}

const char* SUBMISSION_URL_PREFIX = "https://XXX.reddit.com/r/";

int slashindx(const char* str){
    int i = 0;
    while (str[i] != '/')
        ++i;
    return i;
}

void process_submission(const char* url){
    int i = strlen(SUBMISSION_URL_PREFIX);
    
    const int subreddit_len = slashindx(url + i);
    char subreddit[subreddit_len + 1];
    memcpy(subreddit,  url + strlen(SUBMISSION_URL_PREFIX),  subreddit_len);
    subreddit[subreddit_len] = 0;
    i += subreddit_len + 1;
    i += slashindx(url + i) + 1; // Skip the /comments/ section
    
    const int submission_id_len = slashindx(url + i);
    char submission_id[submission_id_len + 1];
    memcpy(submission_id,  url + i,  submission_id_len);
    submission_id[submission_id_len] = 0;
    i += submission_id_len + 1;
    
    
    const char* params = "?limit=2048&sort=best&raw_json=1";
    const int params_len = strlen(params);
    
    
    char api_url[strlen(API_SUBMISSION_URL_PREFIX) + submission_id_len + 1 + params_len + 1];
    int api_url_indx = 0;
    memcpy(api_url + api_url_indx,  API_SUBMISSION_URL_PREFIX,  strlen(API_SUBMISSION_URL_PREFIX));
    api_url_indx += strlen(API_SUBMISSION_URL_PREFIX);
    memcpy(api_url + api_url_indx,  submission_id,  submission_id_len);
    api_url_indx += submission_id_len;
    api_url[api_url_indx++] = '/';
    memcpy(api_url + api_url_indx,  params,  params_len);
    api_url_indx += params_len;
    api_url[api_url_indx] = 0;
    
    request("GET", api_url);
}

int main(const int argc, const char* argv[]){
    MEMORY.memory = (char*)malloc(0);
    
    int i = 0;
    
    const char* usr = argv[++i];
    const char* pwd = argv[++i];
    const char* authstr = argv[++i];
    
    
    login(usr, pwd, authstr);
    
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl)
        handler(ERR_CANNOT_INIT_CURL);
    
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_res_to_mem);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&MEMORY);
    
    curl_slist_append(HEADERS, "Accept-Encoding: gzip, deflate");
    curl_slist_append(HEADERS, "Accept: */*");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, HEADERS);
    
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
    curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, OATH_BEARER_TOKEN);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
    
    
    print_cookies(curl); // TMP
    printf("OATH_BEARER_TOKEN: %s\n", OATH_BEARER_TOKEN); // TMP
    
/*
  #ifdef DEBUG
    if (curl_easy_setopt(curl, CURLOPT_PROXY, "localhost:8000") != CURLE_OK)
        return ERR_CANNOT_SET_PROXY;
  #endif
*/
    
    while (++i < argc){
        sleep(2);
        process_submission(argv[i]);
        print_cookies(curl); // TMP
    }
    
    handler(0);
}
