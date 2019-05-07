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

#ifdef SPIDER
#include <vector> // for std::vector
#include <algorithm> // for std::find
#endif


constexpr const char* URL_PRE  = "https://oauth.reddit.com/r/";
constexpr const char* URL_POST = "/about/moderators/?raw_json=1";
char URL[strlen(URL_PRE) + myru::SUBREDDIT_NAME_MAX + strlen(URL_POST) + 1] = "https://oauth.reddit.com/r/";


constexpr const char* SQL__INSERT_USER_MODDED_SUB_PRE = "INSERT INTO moderator (permissions, added_on, user_id, subreddit_id) VALUES (0,0,";
char SQL__INSERT_USER_MODDED_SUB[strlen(SQL__INSERT_USER_MODDED_SUB_PRE) + 20 + 1 + 20 + 1 + 1] = "INSERT INTO moderator (permissions, added_on, user_id, subreddit_id) VALUES (0,0,";
// NOTE: Do not need 'IGNORE' as this is only called if there were no previous results


std::vector<uint64_t> SUBS_TO_SCRAPE;


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

constexpr const char* SQL__INSERT_USER__PRE  = "INSERT IGNORE INTO user (id, name) VALUES (";
char SQL__INSERT_USER[strlen(SQL__INSERT_USER__PRE) + 20 + 1 + 128 + 1 + 1] = "INSERT IGNORE INTO user (id, name) VALUES (";

constexpr const char* SQL__INSERT_MOD_PRE = "INSERT IGNORE INTO moderator (subreddit_id, user_id, permissions, added_on, rank) VALUES ";
constexpr size_t BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA = 1 + 20 + 1 + 20 + 1 + 20 + 1 + 10 + 1; // Maximum length of a single entry
char* SQL__INSERT_MOD = (char*)malloc(strlen(SQL__INSERT_MOD_PRE) + 100*BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA);
// Some subreddits have thousands of moderators. I do not know the limit.
size_t SQL__INSERT_MOD_SIZE = strlen(SQL__INSERT_MOD_PRE) + 100*BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA;
size_t SQL__INSERT_MOD_INDX;

constexpr const char* SQL__INSERT_SUBREDDIT_NAME_PRE = "INSERT IGNORE INTO subreddit (id, name) VALUES (";
char SQL__INSERT_SUBREDDIT_NAME[strlen(SQL__INSERT_SUBREDDIT_NAME_PRE) + 20+1+128+1 + 1] = "INSERT IGNORE INTO subreddit (id, name) VALUES (";
char SUBREDDIT_NAME[128];
size_t SUBREDDIT_NAME_LEN;


constexpr const char* SQL__GET_SUBREDDIT_ID_PRE = "SELECT id FROM subreddit WHERE name = \"";
char SQL__GET_SUBREDDIT_ID[strlen(SQL__GET_SUBREDDIT_ID_PRE) + 20+1+128+1 + 1] = "SELECT id FROM subreddit WHERE name = \"";



constexpr const char* SQL__SELECT_SUBREDDIT_NAME_PRE = "SELECT name FROM subreddit WHERE id = ";
char SQL__SELECT_SUBREDDIT_NAME[strlen(SQL__SELECT_SUBREDDIT_NAME_PRE) + 20+1+128+1 + 1] = "SELECT name FROM subreddit WHERE id = ";

void sql__resize_insert_mod(){
    SQL__INSERT_MOD_INDX *= 2;
    SQL__INSERT_MOD = (char*)realloc(SQL__INSERT_MOD,  SQL__INSERT_MOD_INDX);
    // TODO: Check for nullptr
}

constexpr const char* SQL__GET_USER_MODDED_SUBS_PRE = "SELECT subreddit_id FROM moderator WHERE user_id = ";
char SQL__GET_USER_MODDED_SUBS[strlen(SQL__GET_USER_MODDED_SUBS_PRE) + 20 + 1] = "SELECT subreddit_id FROM moderator WHERE user_id = ";


constexpr const char* SQL__GET_MODS_OF__PRE  = "SELECT u.id, u.name FROM user u JOIN (SELECT user_id FROM moderator WHERE permissions != 0 AND subreddit_id = ";
constexpr const char* SQL__GET_MODS_OF__POST = ") A ON A.user_id = u.id";
char SQL__GET_MODS_OF[strlen(SQL__GET_MODS_OF__PRE) + 20 + 1] = "SELECT u.id, u.name FROM user u JOIN (SELECT user_id FROM moderator WHERE permissions != 0 AND subreddit_id = ";


bool previously_got_user_modded_subs(const uint64_t user_id){
    auto i = strlen(SQL__GET_USER_MODDED_SUBS_PRE);
    i += itoa_nonstandard(user_id,  SQL__GET_USER_MODDED_SUBS + i);
    SQL__GET_USER_MODDED_SUBS[i] = 0;
    
    PRINTF("SQL__GET_USER_MODDED_SUBS: %s\n", SQL__GET_USER_MODDED_SUBS);
    
    mysu::SQL_RES = mysu::SQL_STMT->executeQuery(SQL__GET_USER_MODDED_SUBS);
    
    if (!mysu::SQL_RES->next())
        return false;
    
    while (true){
        const uint64_t subreddit_id = mysu::SQL_RES->getUInt64(1);
        if (subreddit_id == 0);
            // TODO: Fix the reason some subreddits are 0 (probably 404'd)
        else if (std::find(SUBS_TO_SCRAPE.begin(), SUBS_TO_SCRAPE.end(), subreddit_id) == SUBS_TO_SCRAPE.end())
            SUBS_TO_SCRAPE.push_back(subreddit_id);
        if (!mysu::SQL_RES->next())
            return true;
    }
}

void insert_subreddit_nameid(const char* name,  const uint64_t id){
    auto i = strlen(SQL__INSERT_SUBREDDIT_NAME_PRE);
    i += itoa_nonstandard(id,  SQL__INSERT_SUBREDDIT_NAME + i);
    SQL__INSERT_SUBREDDIT_NAME[i++] = ',';
    SQL__INSERT_SUBREDDIT_NAME[i++] = '"';
    memcpy(SQL__INSERT_SUBREDDIT_NAME + i,  name,  strlen(name));
    i += strlen(name);
    SQL__INSERT_SUBREDDIT_NAME[i++] = '"';
    SQL__INSERT_SUBREDDIT_NAME[i++] = ')';
    SQL__INSERT_SUBREDDIT_NAME[i] = 0;
    
    mysu::SQL_STMT->execute(SQL__INSERT_SUBREDDIT_NAME);
}

void record_user_modded_subreddit(const uint64_t user_id,  const uint64_t subreddit_id,  const char* subreddit_name){
    auto i = strlen(SQL__INSERT_USER_MODDED_SUB_PRE);
    i += itoa_nonstandard(user_id,  SQL__INSERT_USER_MODDED_SUB + i);
    SQL__INSERT_USER_MODDED_SUB[i++] = ',';
    i += itoa_nonstandard(subreddit_id,  SQL__INSERT_USER_MODDED_SUB + i);
    SQL__INSERT_USER_MODDED_SUB[i++] = ')';
    SQL__INSERT_USER_MODDED_SUB[i] = 0;
    
    mysu::SQL_STMT->execute(SQL__INSERT_USER_MODDED_SUB);
    
    
    insert_subreddit_nameid(subreddit_name, subreddit_id);
    
    
    SUBS_TO_SCRAPE.push_back(subreddit_id);
}

void add_user_modded_subs(const char* user_name,  const uint64_t user_id){
    rapidjson::Document d;
    
    goto__getusermoddedsubs:
    
    sleep(myrcu::REDDIT_REQUEST_DELAY);
    myrcu::get_user_moderated_subs(user_name);
    
    if (mycu::MEMORY.memory[0] == '<'){
        // <!doctype html>
        fprintf(stderr, "Reddit returned 404 for /user/%s/moderated_subreddits.json\n");
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
    
    if (SQL__INSERT_MOD_SIZE  <  SQL__INSERT_MOD_INDX + BRCKT_SUBID_COMMA_USERID_COMMA_PERMS_BRCKT_COMMA + 1)
        sql__resize_insert_mod();
    
    auto i = SQL__INSERT_MOD_INDX;
    char* stmt = SQL__INSERT_MOD;
    
    const uint64_t added_on = user["date"].GetFloat(); // Have to, due to .0 ending
    
    stmt[i++] = '(';
    i += itoa_nonstandard(subreddit_id,  stmt + i);
    stmt[i++] = ',';
    i += itoa_nonstandard(user_id,  stmt + i);
    stmt[i++] = ',';
    i += itoa_nonstandard(permissions,  stmt + i);
    stmt[i++] = ',';
    i += itoa_nonstandard(added_on,  stmt + i);
    stmt[i++] = ',';
    i += itoa_nonstandard(rank,  stmt + i);
    stmt[i++] = ')';
    stmt[i++] = ',';
    
    SQL__INSERT_MOD_INDX = i;
    
    
    // TODO: Collect these following small commands into one per subreddit
    i = strlen(SQL__INSERT_USER__PRE);
    i += itoa_nonstandard(user_id,  SQL__INSERT_USER + i);
    SQL__INSERT_USER[i++] = ',';
    SQL__INSERT_USER[i++] = '"';
    memcpy(SQL__INSERT_USER + i,  user_name,  strlen(user_name));
    i += strlen(user_name);
    SQL__INSERT_USER[i++] = '"';
    SQL__INSERT_USER[i++] = ')';
    SQL__INSERT_USER[i] = 0;
    
    PRINTF("SQL__INSERT_USER: %s\n", SQL__INSERT_USER);
    mysu::SQL_STMT->execute(SQL__INSERT_USER);
    
    
    process_mod(subreddit_id, user_id, user_name);
}

void subreddit_id2name(const uint64_t id){
    int i = strlen(SQL__SELECT_SUBREDDIT_NAME_PRE);
    i += itoa_nonstandard(id,  SQL__SELECT_SUBREDDIT_NAME + i);
    SQL__SELECT_SUBREDDIT_NAME[i] = 0;
    
    PRINTF("SQL__SELECT_SUBREDDIT_NAME: %s\n", SQL__SELECT_SUBREDDIT_NAME); // tmp
    
    mysu::SQL_RES = mysu::SQL_STMT->executeQuery(SQL__SELECT_SUBREDDIT_NAME);
    
    if (mysu::SQL_RES->next()){
        const std::string ss = mysu::SQL_RES->getString(1);
        const char* s        = ss.c_str();
        SUBREDDIT_NAME_LEN = strlen(s);
        memcpy(SUBREDDIT_NAME,  s,  SUBREDDIT_NAME_LEN);
        return;
    }
    myrcu::handler(myerr::IMPOSSIBLE);
}

void get_mods_of(const uint64_t subreddit_id){
    int i;
    
    i = strlen(SQL__GET_MODS_OF__PRE);
    i += itoa_nonstandard(subreddit_id,  SQL__GET_MODS_OF + i);
    memcpy(SQL__GET_MODS_OF + i,  SQL__GET_MODS_OF__POST,  strlen(SQL__GET_MODS_OF__POST));
    i += strlen(SQL__GET_MODS_OF__POST);
    SQL__GET_MODS_OF[i] = 0;
    
    mysu::SQL_RES = mysu::SQL_STMT->executeQuery(SQL__GET_MODS_OF);
    
    if (!mysu::SQL_RES->next())
        goto goto__getmodsofnotcached;
    
    while (true){
        const uint64_t user_id = mysu::SQL_RES->getUInt64(1);
        const std::string ss  = mysu::SQL_RES->getString(2);
        const char* user_name = ss.c_str();
        process_mod(subreddit_id, user_id, user_name);
        if (!mysu::SQL_RES->next())
            return;
    }
    
    
    
    goto__getmodsofnotcached:
    
    subreddit_id2name(subreddit_id);
    
    PRINTF("Not cached: %s\n", SUBREDDIT_NAME);
    
    i = strlen(URL_PRE);
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
    
    SQL__INSERT_MOD_INDX = strlen(SQL__INSERT_MOD_PRE);
    
    for (rapidjson::Value::ValueIterator itr = d["data"]["children"].Begin();  itr != d["data"]["children"].End();  ++itr)
        process_mod(subreddit_id, *itr);
    
    SQL__INSERT_MOD[--SQL__INSERT_MOD_INDX] = 0; // Overwrite trailing comma
    
    
    mysu::SQL_STMT->execute(SQL__INSERT_MOD);
}

uint64_t subreddit2id(const char* name){
    auto i = strlen(SQL__GET_SUBREDDIT_ID_PRE);
    memcpy(SQL__GET_SUBREDDIT_ID + i,  name,  strlen(name));
    i += strlen(name);
    SQL__GET_SUBREDDIT_ID[i++] = '"';
    SQL__GET_SUBREDDIT_ID[i] = 0;
    
    PRINTF("SQL__GET_SUBREDDIT_ID: %s\n", SQL__GET_SUBREDDIT_ID); // tmp
    mysu::SQL_RES = mysu::SQL_STMT->executeQuery(SQL__GET_SUBREDDIT_ID);
    
    if (mysu::SQL_RES->next())
        return mysu::SQL_RES->getUInt64(1);
    fprintf(stderr, "Cannot translate subreddit name to ID - subreddit not in subreddit table");
    myrcu::handler(myerr::SUBREDDIT_NOT_IN_DB);
}

int main(const int argc, const char* argv[]){
    mysu::init(argv[1]);  // Init SQL
    myrcu::init(argv[2]); // Init CURL
    
    memcpy(SQL__INSERT_MOD,  SQL__INSERT_MOD_PRE,  strlen(SQL__INSERT_MOD_PRE));
    
    for (auto i = 3;  i < argc;  ++i){
        SUBS_TO_SCRAPE.push_back(subreddit2id(argv[i]));
    }
    
#ifdef SPIDER
    myrcu::init_browser_curl();
#endif
    
    for (auto i = 0;  i < SUBS_TO_SCRAPE.size();  ++i){
        get_mods_of(SUBS_TO_SCRAPE[i]);
    }
    
    return 0;
}
