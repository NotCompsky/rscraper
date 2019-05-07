/*
 * Given a subreddit, record the moderators into database
 * Specifically, for each moderator, record their ID against the subreddit_id in the moderator (i.e. user2subreddit_privileges) table, along with an integer specifying their permissions
 */


#include <string.h> // for memcpy
#include <unistd.h> // for sleep

#include <stdio.h> // for printf // tmp

#include "rapidjson/document.h" // for rapidjson::Document
#include "rapidjson/pointer.h" // for rapidjson::GetValueByPointer
// NOTE: These are to prefer local headers, as rapidjson is a header-only library. This allows easy use of any version of rapidjson, as those provided by repositories might be dated.

#include "rapidjson_utils.h" // for SET_DBG_* macros
#include "utils.h" // for PRINTF macro, sql__file_attr_id, sql__get_id_from_table, sql__insert_into_table_at, count_digits, itoa_nonstandard

#include "error_codes.hpp" // for myerr:*

#include "reddit_utils.hpp" // for myru::*
#include "curl_utils.hpp" // for mycu::*
#include "redditcurl_utils.hpp" // for myrcu::*
#include "sql_utils.hpp" // for mysu::*

#include "filter_user.cpp" // for filter_user::*


constexpr const char* URL_PRE  = "https://oauth.reddit.com/r/";
constexpr const char* URL_POST = "/about/moderators/?raw_json=1";
char URL[strlen(URL_PRE) + myru::SUBREDDIT_NAME_MAX + strlen(URL_POST) + 1] = "https://oauth.reddit.com/r/";


uint64_t calc_permission(const char* str){
    switch(str[0]){
        case 'a':
            switch(str[1]){
                case 'c':
                    return 1; // access
                default:
                    return ~0; // all
            }
        case 'c':
            switch(str[1]){
                case 'h':
                    switch(str[2]){
                        case 'c':
                            return 64; // chat_config
                        default:
                            return 128; // chat_operator
                    }
                default:
                    return 2; // config
            }
        case 'f':
            return 4; // flair
        case 'm':
            return 8; // mail
        case 'p':
            return 16; // posts
        default:
            return 32; // wiki
    }
}

constexpr const char* SQL__INSERT_MOD_PRE = "INSERT INTO moderator (subreddit_id, user_id, permissions) VALUES ";
constexpr size_t BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA = 1 + 20 + 1 + 20 + 1 + 20 + 1; // Maximum length of a single entry
char* SQL__INSERT_MOD = (char*)malloc(strlen(SQL__INSERT_MOD_PRE) + 100*BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA);
// Some subreddits have thousands of moderators. I do not know the limit.
size_t SQL__INSERT_MOD_SIZE = strlen(SQL__INSERT_MOD_PRE) + 100*BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA;
size_t SQL__INSERT_MOD_INDX;

void sql__resize_insert_mod(){
    SQL__INSERT_MOD_INDX *= 2;
    SQL__INSERT_MOD = (char*)realloc(SQL__INSERT_MOD,  SQL__INSERT_MOD_INDX);
    // TODO: Check for nullptr
}

void process_mod(const uint64_t subreddit_id,  rapidjson::Value& user){
    SET_STR(user_id_str,    user["id"]);
    user_id_str += 3; // Skip prefix "t2_"
    const uint64_t user_id = myru::id2n_lower(user_id_str);
    SET_STR(user_name,  user["name"]);
    
    const uint64_t RSET(added_on, user["date"], GetFloat); // Have to convert from float to uint64_t
    
    uint64_t permissions = 0;
    for (rapidjson::Value::ValueIterator itr = user["mod_permissions"].Begin();  itr != user["mod_permissions"].End();  ++itr)
        permissions |= calc_permission(itr->GetString());
    
    if (SQL__INSERT_MOD_SIZE  <  SQL__INSERT_MOD_INDX + BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA + 1)
        sql__resize_insert_mod();
    
    auto i = SQL__INSERT_MOD_INDX;
    char* stmt = SQL__INSERT_MOD;
    
    stmt[i++] = '(';
    i += itoa_nonstandard(subreddit_id,  stmt + i);
    stmt[i++] = ',';
    i += itoa_nonstandard(user_id,  stmt + i);
    stmt[i++] = ',';
    i += itoa_nonstandard(permissions,  stmt + i);
    stmt[i++] = ')';
    stmt[i++] = ',';
    
    SQL__INSERT_MOD_INDX = i;
}

void get_mods_of(const char* subreddit_name){
    int i = strlen(URL_PRE);
    memcpy(URL + i,  subreddit_name,  strlen(subreddit_name));
    i += strlen(subreddit_name);
    memcpy(URL + i,  URL_POST,  strlen(URL_POST));
    i += strlen(URL_POST);
    URL[i] = 0;
    
    mycu::request(URL);
    
    rapidjson::Document d;
    
    if (myrcu::try_again(d)){
        sleep(myrcu::REDDIT_REQUEST_DELAY);
        get_mods_of(subreddit_name);
    }
    
    const uint64_t subreddit_id = mysu::get_subreddit_id(subreddit_name);
    
    if (subreddit_id == 0)
        // Subreddit not found int table
        // TODO: Scrape subreddit details if it does not already exist
        myrcu::handler(myerr::UNKNOWN);
    
    SQL__INSERT_MOD_INDX = strlen(SQL__INSERT_MOD_PRE);
    
    for (rapidjson::Value::ValueIterator itr = d["data"]["children"].Begin();  itr != d["data"]["children"].End();  ++itr)
        process_mod(subreddit_id, *itr);
    
    SQL__INSERT_MOD[--SQL__INSERT_MOD_INDX] = 0; // Overwrite trailing comma
    
    
    printf("%s\n", SQL__INSERT_MOD); // tmp
    
    mysu::SQL_STMT->execute(SQL__INSERT_MOD);
}

int main(const int argc, const char* argv[]){
    mysu::init(argv[1]); // Init SQL
    myrcu::init(argv[3], argv[4], argv[5], argv[2]); // Init CURL
    
    memcpy(SQL__INSERT_MOD,  SQL__INSERT_MOD_PRE,  strlen(SQL__INSERT_MOD_PRE));
    
    for (auto i = 6;  i < argc;  ++i){
        sleep(myrcu::REDDIT_REQUEST_DELAY);
        get_mods_of(argv[i]);
    }
}
