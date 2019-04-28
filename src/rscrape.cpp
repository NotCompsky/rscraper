#include <b64/encode.h> // for base64::encode
#include <curl/curl.h>
#include "rapidjson/document.h" // for rapidjson::Document
#include "rapidjson/pointer.h" // for rapidjson::GetValueByPointer
#include <stdlib.h> // for free, malloc, realloc
#include <string.h> // for memcpy
#include <time.h> // for asctime
#include <unistd.h> // for sleep

/* MySQL */
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "rapidjson_utils.h" // for SET_DBG_* macros
#include "defines.h" // for PRINTF macro
#include "utils.h" // for sql__file_attr_id, sql__get_id_from_table, sql__insert_into_table_at, count_digits, itoa_nonstandard

#include "filter_comment_body.cpp" // for filter_comment_body::*
#include "filter_user.cpp" // for filter_user::*
#include "filter_subreddit.cpp" // for filter_subreddit::*


#ifdef DEBUG
    #include <stdio.h> // for printf
    #include <execinfo.h> // for printing stack trace
#endif


enum {
    SUCCESS,
    ERR,
    ERR_CANNOT_INIT_CURL,
    ERR_CANNOT_WRITE_RES,
    ERR_CURL_PERFORM,
    ERR_CANNOT_SET_PROXY,
    ERR_INVALID_PJ,
    ERR_PARSE
};

#define REDDIT_REQUEST_DELAY 1



sql::Driver* SQL_DRIVER;
sql::Connection* SQL_CON;
sql::Statement* SQL_STMT;
sql::ResultSet* SQL_RES;



const char* USER_AGENT = "rscraper++:0.0.1-dev0 (by /u/Compsky)";
CURL* curl;
const char* PARAMS = "?limit=2048&sort=new&raw_json=1";
const int PARAMS_LEN = strlen(PARAMS);

const char* AUTH_HEADER_PREFIX = "Authorization: bearer ";
const char* TOKEN_FMT = "XXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXXXX";
char AUTH_HEADER[strlen("Authorization: bearer ") + strlen("XXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXXXX") + 1] = "Authorization: bearer ";

const char* API_SUBMISSION_URL_PREFIX = "https://oauth.reddit.com/comments/";
const char* API_DUPLICATES_URL_PREFIX = "https://oauth.reddit.com/duplicates/";
const char* API_SUBREDDIT_URL_PREFIX = "https://oauth.reddit.com/r/";
const char* SUBMISSION_URL_PREFIX = "https://XXX.reddit.com/r/";
const char* API_ALLCOMMENTS_URL = "https://oauth.reddit.com/r/all/comments/?limit=100&raw_json=1";

const char* USR;
const char* PWD;
const char* KEY_AND_SECRET;


const char* BASIC_AUTH_PREFIX = "Authorization: Basic ";
const char* BASIC_AUTH_FMT = "base-64-encoded-client_key:client_secret----------------";
char BASIC_AUTH_HEADER[strlen("Authorization: Basic ") + strlen("base-64-encoded-client_key:client_secret----------------") + 1] = "Authorization: Basic ";


CURL* LOGIN_CURL;
struct curl_slist* LOGIN_HEADERS;
const char* LOGIN_POSTDATA_PREFIX = "grant_type=password&password=";
const char* LOGIN_POSTDATA_KEYNAME = "&username=";
char* LOGIN_POSTDATA;



struct curl_slist* HEADERS;

struct MemoryStruct {
    char* memory;
    size_t size;
    size_t n_allocated;
};

struct MemoryStruct MEMORY;

void handler(int n){
    void* arr[10];

#ifdef DEBUG
    size_t size = backtrace(arr, 10);
    
    fprintf(stderr, "%s\n", MEMORY.memory);
    fprintf(stderr, "E(%d):\n", n);
    backtrace_symbols_fd(arr, size, STDERR_FILENO);
#endif
    
    free(MEMORY.memory);
    free(LOGIN_POSTDATA);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    exit(n);
}


void sql__add_submission_from_cmnt(const unsigned long int id, const unsigned long int subreddit_id, const char is_submission_nsfw){
    /*
    Checks if a submission entry exists, and if not, creates one (but is based only on the information visible from a comment entry)
    */
    int i;
    const char* a = "SELECT id FROM submission WHERE id = ";
    const char* b = "INSERT INTO submission (id, subreddit_id, nsfw) values(";
    char statement[strlen(b) + count_digits(id) + 2 + count_digits(subreddit_id) + 2 + 1];
    
    
    i = 0;
    
    memcpy(statement + i,  a,  strlen(a));
    i += strlen(a);
    
    i += itoa_nonstandard(id, statement + i);
    
    statement[i++] = ';';
    statement[i] = 0;
    
    PRINTF("stmt: %s\n", statement);
    SQL_RES = SQL_STMT->executeQuery(statement);
    
    if (SQL_RES->next()){
        // Entry already existed in table
        return;
    }
    
    
    i = 0;
    
    memcpy(statement + i,  b,  strlen(b));
    i += strlen(b);
    
    i += itoa_nonstandard(id, statement + i);
    
    statement[i++] = ',';
    
    i += itoa_nonstandard(subreddit_id, statement + i);
    
    statement[i++] = ',';
    statement[i++] = '0' + is_submission_nsfw;
    
    statement[i++] = ')';
    statement[i++] = ';';
    statement[i] = 0;
    
    PRINTF("stmt: %s\n", statement);
    SQL_STMT->execute(statement);
}


void sql__add_cmnt(const unsigned long int cmnt_id, const unsigned long int parent_id, const unsigned long int author_id, const unsigned long int submission_id, const unsigned long int created_at, char* content, const unsigned int reason_matched){
    int i;
    const char* a = "SELECT id FROM comment WHERE id = ";
    const char* statement2 = "INSERT INTO comment (id, parent_id, author_id, submission_id, created_at, reason_matched, content) values(";
    char statement[strlen(statement2) + count_digits(cmnt_id) + 2 + count_digits(parent_id) + 2 + count_digits(author_id) + 2 + count_digits(submission_id) + 2 + count_digits(created_at) + 2 + 1 + strlen(content)*2 + 3 + 1];
    
    
    i = 0;
    
    memcpy(statement + i,  a,  strlen(a));
    i += strlen(a);
    
    i += itoa_nonstandard(cmnt_id, statement + i);
    
    statement[i++] = ';';
    statement[i] = 0;
    
    PRINTF("stmt: %s\n", statement);
    SQL_RES = SQL_STMT->executeQuery(statement);
    
    if (SQL_RES->next()){
        // Entry already existed in table
        return;
    }
    
    
    i = 0;
    
    memcpy(statement + i,  statement2,  strlen(statement2));
    i += strlen(statement2);
    
    i += itoa_nonstandard(cmnt_id, statement + i);
    
    statement[i++] = ',';
    statement[i++] = ' ';
    
    i += itoa_nonstandard(parent_id, statement + i);
    
    statement[i++] = ',';
    statement[i++] = ' ';
    
    i += itoa_nonstandard(author_id, statement + i);
    
    statement[i++] = ',';
    statement[i++] = ' ';
    
    i += itoa_nonstandard(submission_id, statement + i);
    statement[i++] = ',';
    statement[i++] = ' ';
    
    i += itoa_nonstandard(created_at, statement + i);
    
    statement[i++] = ',';
    statement[i++] = ' ';
    
    i += itoa_nonstandard(reason_matched,  statement + i);
    
    statement[i++] = ',';
    statement[i++] = ' ';
    
    statement[i++] = '"';
    for (auto j = 0;  j < strlen(content);  ++j){
        if (content[j] == '"'  ||  content[j] == '\\')
            statement[i++] = '\\';
        statement[i++] = content[j];
    }
    statement[i++] = '"';
    
    statement[i++] = ')';
    statement[i++] = ';';
    statement[i] = 0;
    
    PRINTF("stmt: %s\n", statement);
    SQL_STMT->execute(statement);
}





size_t write_res_to_mem(void* content, size_t size, size_t n, void* buf){
    size_t total_size = size * n;
    struct MemoryStruct* mem = (struct MemoryStruct*)buf;
    
    if (mem->size + total_size  >  mem->n_allocated){
        mem->memory = (char*)realloc(mem->memory,  (mem->size + total_size + 1)*3/2);
        mem->n_allocated = (mem->size + total_size + 1)*3/2;
    }
    // Larger requests are not written in just one call
    
    if (!mem->memory)
        handler(ERR_CANNOT_WRITE_RES);
    
    memcpy(mem->memory + mem->size,  content,  total_size);
    mem->size += total_size;
    mem->memory[mem->size] = 0;
    
    return total_size;
}


int request(const char* url){
    PRINTF("GET\t%s\n", url);
    // Writes response contents to MEMORY
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    MEMORY.size = 0; // 'Clear' last request
    
    if (curl_easy_perform(curl) != CURLE_OK)
        handler(ERR_CURL_PERFORM);
}

void init_login(){
    int i;
    
    
    base64::encoder base64_encoder;
    
    i = strlen(BASIC_AUTH_PREFIX);
    
    base64_encoder.encode(KEY_AND_SECRET,  strlen(KEY_AND_SECRET),  BASIC_AUTH_HEADER + i);
    i += strlen(BASIC_AUTH_FMT);
    
    BASIC_AUTH_HEADER[i] = 0;
    
    
    LOGIN_HEADERS = curl_slist_append(LOGIN_HEADERS, BASIC_AUTH_HEADER);
    
    
    LOGIN_CURL = curl_easy_init();
    if (!LOGIN_CURL)
        handler(ERR_CANNOT_INIT_CURL);
    
    curl_easy_setopt(LOGIN_CURL, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(LOGIN_CURL, CURLOPT_HTTPHEADER, LOGIN_HEADERS);
    curl_easy_setopt(LOGIN_CURL, CURLOPT_TIMEOUT, 20);
    
    i = 0;
    
    memcpy(LOGIN_POSTDATA + i,  LOGIN_POSTDATA_PREFIX,  strlen(LOGIN_POSTDATA_PREFIX));
    i += strlen(LOGIN_POSTDATA_PREFIX);
    
    memcpy(LOGIN_POSTDATA + i,  PWD,  strlen(PWD));
    i += strlen(PWD);
    
    memcpy(LOGIN_POSTDATA + i,  LOGIN_POSTDATA_KEYNAME,  strlen(LOGIN_POSTDATA_KEYNAME));
    i += strlen(LOGIN_POSTDATA_KEYNAME);
    
    memcpy(LOGIN_POSTDATA + i,  USR,  strlen(USR));
    i += strlen(USR);
    
    LOGIN_POSTDATA[i] = 0;
    
    
    curl_easy_setopt(LOGIN_CURL, CURLOPT_POSTFIELDS, LOGIN_POSTDATA);
    
    curl_easy_setopt(LOGIN_CURL, CURLOPT_URL, "https://www.reddit.com/api/v1/access_token");
    
    curl_easy_setopt(LOGIN_CURL, CURLOPT_CUSTOMREQUEST, "POST");
    
    curl_easy_setopt(LOGIN_CURL, CURLOPT_WRITEFUNCTION, write_res_to_mem);
    curl_easy_setopt(LOGIN_CURL, CURLOPT_WRITEDATA, (void *)&MEMORY);
}

void login(){
    MEMORY.size = 0; // 'Clear' last request
    
    if (curl_easy_perform(LOGIN_CURL) != CURLE_OK)
        handler(ERR_CURL_PERFORM);
    
    // Result is in format
    // {"access_token": "XXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXXXX", "token_type": "bearer", "expires_in": 3600, "scope": "*"}
    
    int i = strlen(AUTH_HEADER_PREFIX);
    
    memcpy(AUTH_HEADER + i,  MEMORY.memory + strlen("{\"access_token\": \""),  strlen(TOKEN_FMT));
    i += strlen(TOKEN_FMT);
    
    AUTH_HEADER[i] = 0;
    
    
    PRINTF("Response:\n%s\nAUTH_HEADER: %s\n", MEMORY.memory, AUTH_HEADER);
    
    
    HEADERS = {};
    HEADERS = curl_slist_append(HEADERS, AUTH_HEADER);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, HEADERS);
}

int slashindx(const char* str){
    int i = 0;
    while (str[i] != '/')
        ++i;
    return i;
}

unsigned long int id2n_lower(const char* str){
    unsigned long int n = 0;
    //PRINTF("id2n_lower(%s) -> ", str);
    while (*str != 0){
        n *= (10 + 26);
        if (*str >= '0'  &&  *str <= '9')
            n += *str - '0';
#ifdef DEBUG
        else if (*str < 'a'  ||  *str > 'z'){
            fprintf(stderr, "Bad alphanumeric: %s\n", str);
            handler(ERR);
        }
#endif
        else
            n += *str - 'a' + 10;
        ++str;
    }
    //PRINTF("%lu\n", n);
    return n;
}

unsigned long int id2n(const char* str){
    unsigned long n = 0;
    PRINTF("id2n(%s) -> ", str);
    while (*str != 0){
        n *= (10 + 26 + 26);
        if (*str >= '0'  &&  *str <= '9')
            n += *str - '0';
        else if (*str >= 'a'  &&  *str <= 'z')
            n += *str - 'a' + 10;
#ifdef DEBUG
        else if (*str < 'A'  ||  *str > 'Z'){
            fprintf(stderr, "Bad alphanumeric: %s\n", str);
            handler(ERR);
        }
#endif
        else
            n += *str - 'A' + 10 + 26;
        ++str;
    }
    PRINTF("%lu\n", n);
    return n;
}

void count_user_subreddit_cmnt(const unsigned long int user_id,  const unsigned long int subreddit_id){
    int i;
    const char* a = "INSERT INTO user2subreddit_cmnt_count (count, user_id, subreddit_id) VALUES (1,";
    const char* b = ") ON DUPLICATE KEY UPDATE count = count + 1;";
    char stmt[strlen(a) + count_digits(user_id) + 1 + count_digits(subreddit_id) + strlen(b) + 1];
    
    
    i = 0;
    
    memcpy(stmt + i,  a,  strlen(a));
    i += strlen(a);
    
    i += itoa_nonstandard(user_id,  stmt + i);
    
    stmt[i++] = ',';
    
    i += itoa_nonstandard(subreddit_id,  stmt + i);
    
    memcpy(stmt + i,  b,  strlen(b));
    i += strlen(b);
    
    stmt[i] = 0;
    
    
    PRINTF("stmt: %s\n", stmt);
    SQL_STMT->execute(stmt);
}

void process_live_cmnt(rapidjson::Value& cmnt, const unsigned long int cmnt_id){
    SET_STR(body,           cmnt["data"]["body"]);
    SET_STR(subreddit_name, cmnt["data"]["subreddit"]);
    SET_STR(author_name,    cmnt["data"]["author"]);
    
    
    const unsigned long int author_id = id2n_lower(cmnt["data"]["author_fullname"].GetString() + 3); // Skip "t2_" prefix
    const unsigned long int subreddit_id = id2n_lower(cmnt["data"]["subreddit_id"].GetString() + 3); // Skip "t3_" prefix
    const bool is_submission_nsfw = cmnt["data"]["over_18"].GetBool();
    char is_subreddit_nsfw = 2; // 0 for certainly SFW, 1 for certainly NSFW. 2 for unknown.
    
    if (!is_submission_nsfw)
        is_subreddit_nsfw = 0;
    
    
    if (filter_subreddit::to_count(subreddit_id))
        count_user_subreddit_cmnt(author_id, subreddit_id);
    
    
    struct cmnt_meta metadata = {
        author_name,
        subreddit_name,
        
        author_id,
        subreddit_id,
    };
    
    
    switch (filter_user::matches_id(author_id)){
        case 1: return; // blacklist
        case 2: goto goto__do_process_this_live_cmnt; // whitelist
        default: break; // 0 is the default
    }
    
    switch (filter_subreddit::matches_id(subreddit_id)){
        case 1: return; // blacklist
        case 2: goto goto__do_process_this_live_cmnt; // whitelist
        default: break; // 0 is the default
    }
    
    
    unsigned int reason_matched;
    if ((reason_matched = filter_comment_body::wl::match(metadata, body, strlen(body)))){
        PRINTF("MATCHED: %s\n", filter_comment_body::wl::what[0].str().c_str());
        goto goto__do_process_this_live_cmnt;
    }
    // if filter_comment_body::bl: return;
    
    
    
    return;
    
    goto__do_process_this_live_cmnt:
    
    
    SET_STR(permalink,      cmnt["data"]["permalink"]);
    
    const time_t RSET_FLT(created_at,     cmnt["data"]["created_utc"]);
    
    
    sql__insert_into_table_at(SQL_STMT, SQL_RES, "user", author_name, author_id);
    sql__insert_into_table_at(SQL_STMT, SQL_RES, "subreddit", subreddit_name, subreddit_id);
    
    unsigned long int parent_id = id2n_lower(cmnt["data"]["parent_id"].GetString() + 3);
    unsigned long int submission_id;
    
    if (cmnt["data"]["parent_id"].GetString()[1] == '3'){
        // "t3_" or "t1_" prefix
        submission_id = parent_id;
        parent_id = 0;
    } else {
        submission_id = id2n_lower(cmnt["data"]["link_id"].GetString() + 3);
    }
    
    const char* cmnt_content = cmnt["data"]["body"].GetString();
    sql__add_cmnt(cmnt_id, parent_id, author_id, submission_id, created_at, (char*)cmnt_content, reason_matched);
    sql__add_submission_from_cmnt(submission_id, subreddit_id, is_submission_nsfw);
}

unsigned long int process_live_replies(rapidjson::Value& replies, const unsigned long int last_processed_cmnt_id){
    /*
    'replies' object is the 'replies' JSON object which has property 'kind' of value 'Listing'
    */
    unsigned long int cmnt_id;
    int i = 0;
    
    for (rapidjson::Value::ValueIterator itr = replies["data"]["children"].Begin();  itr != replies["data"]["children"].End();  ++itr){
        cmnt_id = id2n_lower((*itr)["data"]["id"].GetString()); // No "t1_" prefix
        if (cmnt_id <= last_processed_cmnt_id)
            // Not '==' since it is possible for comments to have been deleted between calls
            break;
        process_live_cmnt(*itr, cmnt_id);
        ++i;
    }
    
    return id2n_lower(replies["data"]["children"][0]["data"]["id"].GetString());
}


const char* FORBIDDEN = ">403 Forbidden<";

bool try_again(rapidjson::Document& d){
    if (d.Parse(MEMORY.memory).HasParseError())
        handler(ERR_PARSE);
    
    if (!d.HasMember("error"))
        return false;
    else {
        switch (d["error"].GetInt()){
            case 401:
                // Unauthorised
                PRINTF("Unauthorised. Logging in again.\n");
                sleep(REDDIT_REQUEST_DELAY);
                login();
                break;
            default:
                handler(ERR);
        }
        return true;
    }
}

void process_all_comments_live(){
    unsigned long int last_processed_cmnt_id = 0;
    
    while (true){
        sleep(REDDIT_REQUEST_DELAY);
        
        
        request(API_ALLCOMMENTS_URL);
        
        rapidjson::Document d;
        
        if (try_again(d))
            continue;
        
        last_processed_cmnt_id = process_live_replies(d, last_processed_cmnt_id);
    }
}

void process_moderator(rapidjson::Value& user){
    SET_STR(user_id,    user["id"]);
    user_id += 3; // Skip prefix "t2_"
    SET_STR(user_name,  user["name"]);
    
    const size_t RSET(added_on, user["date"], GetFloat);
    
    // TODO: process mod_permissions, converting array of strings like "all" to integer of bits
}

void process_moderators(const char* subreddit, const int subreddit_len){
    const char* a = "/about/moderators/?raw_json=1";
    char api_url[strlen(API_SUBREDDIT_URL_PREFIX) + subreddit_len + strlen(a) + 1];
    int i = 0;
    
    
    memcpy(api_url + i,  API_SUBREDDIT_URL_PREFIX,  strlen(API_SUBREDDIT_URL_PREFIX));
    i += strlen(API_SUBREDDIT_URL_PREFIX);
    
    memcpy(api_url + i,  subreddit,  subreddit_len);
    i += subreddit_len;
    
    memcpy(api_url + i,  a,  strlen(a));
    i += strlen(a);
    
    api_url[i] = 0;
    
    
    request(api_url);
    
    rapidjson::Document d;
    if (d.Parse(MEMORY.memory).HasParseError())
        handler(ERR_INVALID_PJ);
    
    
    for (rapidjson::Value::ValueIterator itr = d["data"]["children"].Begin();  itr != d["data"]["children"].End();  ++itr)
        process_moderator(*itr);
    
    
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

int main(const int argc, const char* argv[]){
    MEMORY.memory = (char*)malloc(0);
    MEMORY.n_allocated = 0;
    
    
    PRINTF("filter_comment_body::wl::regexpr_str: %s\n", filter_comment_body::wl::regexpr_str);
    
    
    int i = 0;
    
    
    SQL_DRIVER = get_driver_instance();
    SQL_CON = SQL_DRIVER->connect(argv[1], argv[2], argv[3]);
    i += 3;
    SQL_CON->setSchema("rscraper");
    SQL_STMT = SQL_CON->createStatement();
    
    
    USR = argv[++i];
    PWD = argv[++i];
    KEY_AND_SECRET = argv[++i];
    
    LOGIN_POSTDATA = (char*)malloc(strlen(LOGIN_POSTDATA_PREFIX) + strlen(PWD) + strlen(LOGIN_POSTDATA_KEYNAME) + strlen(USR) + 1);
    
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl)
        handler(ERR_CANNOT_INIT_CURL);
    
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
    
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_res_to_mem);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&MEMORY);
    
    
    init_login();
    login();
    
    while (++i < argc){
        sleep(REDDIT_REQUEST_DELAY);
        process_submission(argv[i]);
    }
    
    process_all_comments_live();
}
