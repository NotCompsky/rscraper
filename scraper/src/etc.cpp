/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


// Unused code

constexpr static const char* API_SUBMISSION_URL_PREFIX = "https://oauth.reddit.com/comments/";
constexpr static const char* SUBMISSION_URL_PREFIX = "https://XXX.reddit.com/r/";
constexpr static const char* API_DUPLICATES_URL_PREFIX = "https://oauth.reddit.com/duplicates/";
constexpr static const char* API_SUBREDDIT_URL_PREFIX = "https://oauth.reddit.com/r/";

constexpr static const char* PARAMS = "?limit=2048&sort=new&raw_json=1";
constexpr static const size_t PARAMS_LEN = strlen_constexpr(PARAMS);

int slashindx(const char* str){
    int i = 0;
    while (str[i] != '/')
        ++i;
    return i;
}

void process_submission_duplicates(const char* submission_id, const int submission_id_len){
    int i = 0;
    char api_url[strlen(API_DUPLICATES_URL_PREFIX) + submission_id_len + 1 + PARAMS_LEN + 1];
    
    memcpy(api_url + i,  API_DUPLICATES_URL_PREFIX,  strlen(API_DUPLICATES_URL_PREFIX));
    i += strlen(API_DUPLICATES_URL_PREFIX);
    
    memcpy(api_url + i,  submission_id,  submission_id_len);
    i += submission_id_len;
    
    api_url[i++] = '/';
    
    // We only need "?limit=1000&raw_json=1", but the additional parameter "&sort=best" has no effect
    memcpy(api_url + i,  PARAMS,  PARAMS_LEN);
    i += PARAMS_LEN;
    
    api_url[i] = 0;
    
    
    request(api_url);
    
    PRINTF("%s\n", MEMORY.memory);
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
    
    
    char api_url[strlen(API_SUBMISSION_URL_PREFIX) + submission_id_len + 1 + PARAMS_LEN + 1];
    int api_url_indx = 0;
    memcpy(api_url + api_url_indx,  API_SUBMISSION_URL_PREFIX,  strlen(API_SUBMISSION_URL_PREFIX));
    api_url_indx += strlen(API_SUBMISSION_URL_PREFIX);
    memcpy(api_url + api_url_indx,  submission_id,  submission_id_len);
    api_url_indx += submission_id_len;
    api_url[api_url_indx++] = '/';
    memcpy(api_url + api_url_indx,  PARAMS,  PARAMS_LEN);
    api_url_indx += PARAMS_LEN;
    api_url[api_url_indx] = 0;
    
    request(api_url);
    
    
    rapidjson::Document d;
    if (d.Parse(MEMORY.memory).HasParseError())
        handler(ERR_INVALID_PJ);
    
    SET_STR(id,             d[0]["data"]["children"][0]["data"]["id"]);
    // No prefix to ignore
}
