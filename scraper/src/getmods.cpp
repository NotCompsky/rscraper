/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


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

#include "error_codes.hpp" // for myerr:*

#include "reddit_utils.hpp" // for myru::*
#include "curl_utils.hpp" // for mycu::*
#include "redditcurl_utils.hpp" // for myrcu::*

#include "filter_user.hpp" // for filter_user::*

#include <compsky/mysql/query.hpp> // for compsky::mysql::*

#ifdef SPIDER
#include <vector> // for std::vector
#include <algorithm> // for std::find
#endif


MYSQL_RES* RES;
MYSQL_ROW ROW;


constexpr size_t strlen_constexpr(const char* s){
    // GCC strlen is constexpr; this is apparently a bug
    return *s  ?  1 + strlen_constexpr(s + 1)  :  0;
}


constexpr const char* URL_PRE  = "https://oauth.reddit.com/r/";
constexpr const char* URL_POST = "/about/moderators/?raw_json=1";
char URL[strlen(URL_PRE) + myru::SUBREDDIT_NAME_MAX + strlen(URL_POST) + 1] = "https://oauth.reddit.com/r/";




std::vector<uint64_t> SUBS_TO_SCRAPE;
std::vector<unsigned int> DEPTHS;
// Does not make sense to tie the two together into tuples of (SUBREDDIT, DEPTH), as we are often looking through the entirety of SUBS_TO_SCRAPE in order to avoid duplicate subs
unsigned int DEPTH;
unsigned int MAX_DEPTH;


uint64_t calc_permission(const char* str){
    switch(str[0]){
        case 'a':
            switch(str[1]){
                case 'c':
                    return 1; // access
                default:
                    return ~0; // all // Most likely
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

constexpr const char* SQL__INSERT_MOD_PRE = "INSERT IGNORE INTO moderator (subreddit_id, user_id, permissions, added_on, rank) VALUES ";
constexpr const char* SQL__INSERT_MOD_POST = " ON DUPLICATE KEY UPDATE rank=VALUES(rank), permissions=VALUES(permissions)";
constexpr size_t BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA = 1 + 20 + 1 + 20 + 1 + 20 + 1 + 10 + 1; // Maximum length of a single entry
char SQL__INSERT_MOD[strlen_constexpr(SQL__INSERT_MOD_PRE) + 100*BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA + strlen(SQL__INSERT_MOD_POST) + 1];
// Some subreddits have thousands of moderators. I do not know the limit.
size_t SQL__INSERT_MOD_INDX;

char SUBREDDIT_NAME[128];
size_t SUBREDDIT_NAME_LEN;


void sql__flush_insert_mod(){
    compsky::mysql::exec_buffer(SQL__INSERT_MOD, SQL__INSERT_MOD_INDX);
    SQL__INSERT_MOD_INDX = strlen_constexpr(SQL__INSERT_MOD_PRE);
}


unsigned int ascii2n(const char* str){
    unsigned int n = 0;
    for(char* s = const_cast<char*>(str);  *s != 0;  ++s){
        n *= 10;
        n += *s - '0';
    }
};


bool previously_got_user_modded_subs(const uint64_t user_id){
    compsky::mysql::query(&RES, "SELECT subreddit_id FROM moderator WHERE user_id=",  user_id);
    
    uint64_t subreddit_id = 0;
    while(compsky::mysql::assign_next_row(RES, &ROW, &subreddit_id)){
        if (std::find(SUBS_TO_SCRAPE.begin(), SUBS_TO_SCRAPE.end(), subreddit_id) == SUBS_TO_SCRAPE.end()){
            SUBS_TO_SCRAPE.push_back(subreddit_id);
            DEPTHS.push_back(DEPTH + 1);
        }
    }
    
    return (subreddit_id);
}

void record_user_modded_subreddit(const uint64_t user_id,  const uint64_t subreddit_id,  const char* subreddit_name){
    compsky::mysql::exec("INSERT INTO moderator (permissions, added_on, rank, user_id, subreddit_id) VALUES (0,0,0,",  user_id,  ',',  subreddit_id,  ")");
    // NOTE: Do not need 'IGNORE' as this is only called if there were no previous results
    
    compsky::mysql::exec("INSERT IGNORE INTO subreddit (id, name) VALUES (",  subreddit_id, ",\"", subreddit_name, "\")");
    
    SUBS_TO_SCRAPE.push_back(subreddit_id);
    DEPTHS.push_back(DEPTH + 1);
}

void add_user_modded_subs(const char* user_name,  const uint64_t user_id){
    rapidjson::Document d;
    
    goto__getusermoddedsubs:
    
    sleep(myrcu::REDDIT_REQUEST_DELAY);
    myrcu::get_user_moderated_subs(user_name);
    
    if (mycu::MEMORY.memory[0] == '<'){
        // <!doctype html>
        fprintf(stderr, "Reddit returned 404 for /user/%s/moderated_subreddits.json\n", user_name);
        return;
    }
    
    if (myrcu::try_again(d))
        goto goto__getusermoddedsubs;
    
    for (rapidjson::Value::ValueIterator itr = d["data"].Begin();  itr != d["data"].End();  ++itr){
        const uint64_t subreddit_id = myru::id2n_lower((*itr)["name"].GetString() + 3); // Skip t5_ prefix
        if (subreddit_id == 0)
            // TODO: Fix the reason some subreddits are 0 (probably 404'd)
            continue;
        const char* subreddit_name  = (*itr)["sr"].GetString();
        if (std::find(SUBS_TO_SCRAPE.begin(), SUBS_TO_SCRAPE.end(), subreddit_id) == SUBS_TO_SCRAPE.end())
            record_user_modded_subreddit(user_id, subreddit_id, subreddit_name);
    }
}

void process_mod(const uint64_t subreddit_id,  const uint64_t user_id,  const char* user_name){
  #ifdef SPIDER
    if (filter_user::matches_id(user_id))
        // It is a **very** good idea to filter out AutoModerator
        return;
    
    if (previously_got_user_modded_subs(user_id))
        return;
    
    add_user_modded_subs(user_name, user_id);
  #endif
}

void process_mod(const uint64_t subreddit_id,  rapidjson::Value& user,  unsigned int rank){
    SET_STR(user_id_str,    user["id"]);
    user_id_str += 3; // Skip prefix "t2_"
    const uint64_t user_id = myru::id2n_lower(user_id_str);
    SET_STR(user_name,  user["name"]);
    
    uint64_t permissions = 0;
    for (rapidjson::Value::ValueIterator itr = user["mod_permissions"].Begin();  itr != user["mod_permissions"].End();  ++itr)
        permissions |= calc_permission(itr->GetString());
    
    if (sizeof(SQL__INSERT_MOD)  <  SQL__INSERT_MOD_INDX + BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA + 1)
        sql__flush_insert_mod();
    
    auto i = SQL__INSERT_MOD_INDX;
    char* stmt = SQL__INSERT_MOD;
    
    const uint64_t added_on = user["date"].GetFloat(); // Have to, due to .0 ending
    
    constexpr static const compsky::asciify::flag::ChangeBuffer change_buf;
    
    compsky::asciify::asciify(change_buf, SQL__INSERT_MOD, SQL__INSERT_MOD_INDX, '(', subreddit_id, ',', user_id, ',', permissions, ',', added_on, ',', rank, ')', ',');
    
    SQL__INSERT_MOD_INDX = compsky::asciify::BUF_INDX;
    
    // TODO: Collect these following small commands into one per subreddit
    compsky::mysql::exec("INSERT IGNORE INTO user (id, name) VALUES (",  user_id,  ",\"",  user_name,  "\")");
    
    process_mod(subreddit_id, user_id, user_name);
}

void subreddit_id2name(const uint64_t id){
    compsky::mysql::query(&RES, "SELECT name FROM subreddit WHERE id=",  id);
    
    char* s = nullptr;
    while(compsky::mysql::assign_next_row(RES, &ROW, &s)){
        SUBREDDIT_NAME_LEN = strlen(s);
        memcpy(SUBREDDIT_NAME,  s,  SUBREDDIT_NAME_LEN);
    }
    
    if (s == nullptr)
        myrcu::handler(myerr::IMPOSSIBLE);
    
    return;
}

void get_mods_of(const uint64_t subreddit_id){
    compsky::mysql::query(&RES,  "SELECT u.id, u.name FROM user u JOIN (SELECT user_id FROM moderator WHERE permissions != 0 AND subreddit_id=",  subreddit_id,  ") A ON A.user_id = u.id");
    
    uint64_t user_id = 0;
    char* user_name;
    while(compsky::mysql::assign_next_row(RES, &ROW, &user_name)){
        process_mod(subreddit_id, user_id, user_name);
    }
    
    subreddit_id2name(subreddit_id);
    
    PRINTF("Not cached: %s\n", SUBREDDIT_NAME);
    
    auto i = strlen(URL_PRE);
    memcpy(URL + i,  SUBREDDIT_NAME,  SUBREDDIT_NAME_LEN);
    i += SUBREDDIT_NAME_LEN;
    memcpy(URL + i,  URL_POST,  strlen(URL_POST));
    i += strlen(URL_POST);
    URL[i] = 0;
    
    goto_getmodsoftryagain:
    
    sleep(myrcu::REDDIT_REQUEST_DELAY);
    mycu::request(URL);
    
    if (mycu::MEMORY.memory[0] == '<'){
        // <!doctype html>
        fprintf(stderr, "Reddit returned 404 for %s\n", URL);
        return;
    }
    
    rapidjson::Document d;
    
    if (myrcu::try_again(d))
        goto goto_getmodsoftryagain;
    
    SQL__INSERT_MOD_INDX = strlen_constexpr(SQL__INSERT_MOD_PRE);
    
    unsigned int rank = 0;
    for (rapidjson::Value::ValueIterator itr = d["data"]["children"].Begin();  itr != d["data"]["children"].End();  ++itr)
        process_mod(subreddit_id, *itr, ++rank);
    
    memcpy(SQL__INSERT_MOD + SQL__INSERT_MOD_INDX,  SQL__INSERT_MOD_POST,  strlen(SQL__INSERT_MOD_POST));
    i += strlen(SQL__INSERT_MOD_POST);
    --SQL__INSERT_MOD_INDX; // Ignore trailing comma
    
    
    compsky::mysql::exec_buffer(SQL__INSERT_MOD, SQL__INSERT_MOD_INDX);
}

uint64_t subreddit2id(const char* name){
    compsky::mysql::query(&RES, "SELECT id FROM subreddit WHERE name=\"",  name,  "\"");
    
    uint64_t subreddit_id = 0;
    while(compsky::mysql::assign_next_row(RES, &ROW, &subreddit_id));
    
    if (subreddit_id != 0)
        return subreddit_id;
    
    fprintf(stderr, "Cannot translate subreddit name to ID - subreddit not in subreddit table");
    myrcu::handler(myerr::SUBREDDIT_NOT_IN_DB);
}

int main(const int argc,  const char** argv){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));  // Init SQL
    mycu::init();         // Init CURL
    myrcu::init(getenv("RSCRAPER_REDDIT_CFG")); // Init OAuth
    
    MAX_DEPTH = ascii2n(argv[1]);
    
    memcpy(SQL__INSERT_MOD,  SQL__INSERT_MOD_PRE,  strlen_constexpr(SQL__INSERT_MOD_PRE));
    
    for (auto i = 2;  i < argc;  ++i){
        SUBS_TO_SCRAPE.push_back(subreddit2id(argv[i]));
        DEPTHS.push_back(0);
    }
    
#ifdef SPIDER
    myrcu::init_browser_curl();
#endif
    
    for (auto i = 0;  i < SUBS_TO_SCRAPE.size();  ++i){
        DEPTH = DEPTHS[i];
        get_mods_of(SUBS_TO_SCRAPE[i]);
    }
    
    return 0;
}
