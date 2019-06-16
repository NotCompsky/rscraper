// Unused code

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
